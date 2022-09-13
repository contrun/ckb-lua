-- tool functions
function read(v)
end

function Script(v)
    return v
end

function Cell(v)
    return v
end

function Witness(v)
end

function blake2b(v)
    return v
end

-- convert this lua file to json file used by ckb-debugger
tx = {}

tx.cell_deps = {
    name1 = Cell {data = read("./build/risc-v-binary")},
    -- full name
    name2 = Cell {
        capacity = 0x4b9f96b00,
        lock = Script {
            args = "0x",
            code_hash = "0x000...0011",
            hash_type = "data1"
        },
        type = Script {
            args = "0x",
            code_hash = "0x0000...0002",
            hash_type = "type"
        },
        data = "0x0000...1122"
    }
}

tx.inputs = {
    [1] = Cell {
        capacity = 1000000000,
        -- "main" code_alias is a special one. It will be replace by binary specified from ckb-debugger command "--bin" option.
        lock = Script {code_alias = "main", args = "0x00"}
    },
    [2] = Cell {
        capacity = 200000000,
        -- "name1" is defined in cell_deps. The hash_type, outpoint are generated automatically.
        lock = Script {code_alias = "name1", args = "0x01"}
    },
    -- a full example of input cell
    [3] = Cell {
        capacity = 300000,
        lock = Script {
            -- manually get the hash
            code_hash = blake2b(tx.cell_deps["name2"].type),
            hash_type = "type",
            args = "0x0000...0003"
        },
        type = Script {
            -- manually get the hash
            code_hash = blake2b(tx.cell_deps["name2"].data),
            hash_type = "data1",
            args = "0x0000...0004"
        },
        -- use an array as data
        data = "0x0000...0005"
    }
}

tx.outputs = {
    [1] = Cell {
        type = Script {code_alias = "name2", args = "0x02"},
        -- Output lock scripts aren't run. They can be ignored safely.
        data = "0x0000...0005"
    }
}

tx.witnesses = {
    [1] = Witness {lock = "0x", input_type = "0x", output_type = "0x"}
}

return tx
