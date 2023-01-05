use super::{DummyDataLoader, DYLIB_TEST_BIN, LIB_CKB_LUA_BIN, MAX_CYCLES};
use bytes::BufMut;
use ckb_chain_spec::consensus::{Consensus, ConsensusBuilder};

use ckb_script::{TransactionScriptsVerifier, TxVerifyEnv};
use ckb_types::core::hardfork::HardForkSwitch;
use ckb_types::{
    bytes::Bytes,
    bytes::BytesMut,
    core::{
        cell::{CellMetaBuilder, ResolvedTransaction},
        Capacity, DepType, EpochNumberWithFraction, HeaderView, ScriptHashType, TransactionBuilder,
        TransactionView,
    },
    packed::{Byte32, CellDep, CellOutput, OutPoint, Script, WitnessArgsBuilder},
    prelude::*,
};

use rand::rngs::StdRng;
use rand::{Rng, SeedableRng};

fn debug_printer(script: &Byte32, msg: &str) {
    let slice = script.as_slice();
    let _str = format!(
        "Script({:x}{:x}{:x}{:x}{:x})",
        slice[0], slice[1], slice[2], slice[3], slice[4]
    );
    // println!("{:?}: {}", str, msg);
    print!("{msg}");
}

fn gen_tx(dummy: &mut DummyDataLoader) -> TransactionView {
    let mut rng = <StdRng as SeedableRng>::from_seed([42u8; 32]);

    // setup lib_ckb_lua dep
    let lib_ckb_lua_out_point = {
        let contract_tx_hash = {
            let mut buf = [0u8; 32];
            rng.fill(&mut buf);
            buf.pack()
        };
        OutPoint::new(contract_tx_hash, 0)
    };
    // dep contract code
    let lib_ckb_lua_cell = CellOutput::new_builder()
        .capacity(
            Capacity::bytes(LIB_CKB_LUA_BIN.len())
                .expect("script capacity")
                .pack(),
        )
        .build();
    let lib_ckb_lua_cell_data_hash = CellOutput::calc_data_hash(&LIB_CKB_LUA_BIN);
    dummy.cells.insert(
        lib_ckb_lua_out_point.clone(),
        (lib_ckb_lua_cell, LIB_CKB_LUA_BIN.clone()),
    );

    // setup dylib_test dep
    let dylib_test_out_point = {
        let contract_tx_hash = {
            let mut buf = [0u8; 32];
            rng.fill(&mut buf);
            buf.pack()
        };
        OutPoint::new(contract_tx_hash, 0)
    };
    // dep contract code
    let dylib_test_cell = CellOutput::new_builder()
        .capacity(
            Capacity::bytes(DYLIB_TEST_BIN.len())
                .expect("script capacity")
                .pack(),
        )
        .build();
    let dylib_test_cell_data_hash = CellOutput::calc_data_hash(&DYLIB_TEST_BIN);
    dummy.cells.insert(
        dylib_test_out_point.clone(),
        (dylib_test_cell, DYLIB_TEST_BIN.clone()),
    );

    // setup default tx builder
    let dummy_capacity = Capacity::shannons(42);
    let mut tx_builder = TransactionBuilder::default()
        .cell_deps(vec![
            CellDep::new_builder()
                .out_point(dylib_test_out_point)
                .dep_type(DepType::Code.into())
                .build(),
            CellDep::new_builder()
                .out_point(lib_ckb_lua_out_point)
                .dep_type(DepType::Code.into())
                .build(),
        ])
        .output(
            CellOutput::new_builder()
                .capacity(dummy_capacity.pack())
                .build(),
        )
        .output_data(Bytes::new().pack());

    let previous_tx_hash = {
        let mut buf = [0u8; 32];
        rng.fill(&mut buf);
        buf.pack()
    };
    let out_point = OutPoint::new(previous_tx_hash, 0);

    let mut buf = BytesMut::with_capacity(2 + lib_ckb_lua_cell_data_hash.as_slice().len() + 1);
    buf.extend_from_slice(&[0x00u8; 2]);
    buf.extend_from_slice(lib_ckb_lua_cell_data_hash.as_slice());
    buf.put_u8(ScriptHashType::Data1.into());
    let args = buf.freeze();

    let script = Script::new_builder()
        .args(args.pack())
        .code_hash(dylib_test_cell_data_hash)
        .hash_type(ScriptHashType::Data1.into())
        .build();
    let output_cell = CellOutput::new_builder()
        .capacity(dummy_capacity.pack())
        .type_(Some(script).pack())
        .build();
    dummy
        .cells
        .insert(out_point, (output_cell.clone(), Bytes::new()));
    let mut random_extra_witness = [0u8; 32];
    rng.fill(&mut random_extra_witness);
    let witness_args = WitnessArgsBuilder::default()
        .output_type(Some(Bytes::from(random_extra_witness.to_vec())).pack())
        .build();
    tx_builder = tx_builder
        .output(output_cell)
        .witness(witness_args.as_bytes().pack());

    tx_builder.build()
}

pub fn gen_tx_env() -> TxVerifyEnv {
    let epoch = EpochNumberWithFraction::new(300, 0, 1);
    let header = HeaderView::new_advanced_builder()
        .epoch(epoch.pack())
        .build();
    TxVerifyEnv::new_commit(&header)
}

pub fn gen_consensus() -> Consensus {
    let hardfork_switch = HardForkSwitch::new_without_any_enabled()
        .as_builder()
        .rfc_0032(200)
        .build()
        .unwrap();
    ConsensusBuilder::default()
        .hardfork_switch(hardfork_switch)
        .build()
}

fn build_resolved_tx(data_loader: &DummyDataLoader, tx: &TransactionView) -> ResolvedTransaction {
    let resolved_cell_deps = tx
        .cell_deps()
        .into_iter()
        .map(|deps_out_point| {
            let (dep_output, dep_data) =
                data_loader.cells.get(&deps_out_point.out_point()).unwrap();
            CellMetaBuilder::from_cell_output(dep_output.to_owned(), dep_data.to_owned())
                .out_point(deps_out_point.out_point())
                .build()
        })
        .collect();

    let mut resolved_inputs = Vec::new();
    for i in 0..tx.inputs().len() {
        let previous_out_point = tx.inputs().get(i).unwrap().previous_output();
        let (input_output, input_data) = data_loader.cells.get(&previous_out_point).unwrap();
        resolved_inputs.push(
            CellMetaBuilder::from_cell_output(input_output.to_owned(), input_data.to_owned())
                .out_point(previous_out_point)
                .build(),
        );
    }

    ResolvedTransaction {
        transaction: tx.clone(),
        resolved_cell_deps,
        resolved_inputs,
        resolved_dep_groups: vec![],
    }
}

#[test]
fn run_dylib_test() {
    let mut data_loader = DummyDataLoader::new();
    let tx = gen_tx(&mut data_loader);
    let resolved_tx = build_resolved_tx(&data_loader, &tx);
    let consensus = gen_consensus();
    let tx_env = gen_tx_env();
    let mut verifier =
        TransactionScriptsVerifier::new(&resolved_tx, &consensus, &data_loader, &tx_env);
    verifier.set_debug_printer(debug_printer);
    let verify_result = verifier.verify(MAX_CYCLES);
    verify_result.expect("pass verification");
}
