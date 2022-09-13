

CKB-DEBUGGER ?= ckb-debugger
PORT ?= 9999
TEST ?= tmp
MAX-CYCLES ?= 99999999999

define run
	RUST_LOG=debug $(CKB-DEBUGGER) --max-cycles $(MAX-CYCLES) --read-file $(1)  --bin ../../build/lua-loader.debug -- lua-loader -r   
endef

define run_ci
	RUST_LOG=debug $(CKB-DEBUGGER) --max-cycles $(MAX-CYCLES) --read-file $(1)  --bin ../../build/lua-loader.debug -- lua-loader -r  2>&1 | fgrep 'Run result: 0'
endef

define run_with_mocked_tx
	RUST_LOG=debug $(CKB-DEBUGGER) --max-cycles $(MAX-CYCLES) --tx-file sample_data1.json --script-group-type=type --script-hash=0xca505bee92c34ac4522d15da2c91f0e4060e4540f90a28d7202df8fe8ce930ba --read-file $(1)  --bin ../../build/lua-loader.debug -- lua-loader -r  2>&1 | fgrep 'Run result: 0'
endef

define run_pretty_result
	RUST_LOG=debug $(CKB-DEBUGGER) --tx-file sample_data1.json --script-group-type=type --script-hash=0xca505bee92c34ac4522d15da2c91f0e4060e4540f90a28d7202df8fe8ce930ba --read-file $(1)  --bin ../../build/lua-loader.debug -- lua-loader -r |& sed 's/DEBUG.*SCRIPT>//g' | sed ':a;N;$!ba;s/\n/NEWLINE/g' | sed 's/NEWLINENEWLINE/\n/g' | sed 's/NEWLINE//g'
endef

evaluate-lua-code-in-cell-data:
	RUST_LOG=debug $(CKB-DEBUGGER) --tx-file lua_code_in_cell_data.json --script-group-type=type --cell-index=0 --cell-type=output --bin ../../build/lua-loader.debug -- lua-loader 2>&1 | fgrep 'hello world'

all:
	$(call run, test_require.lua)
	$(call run, test_loadfile.lua)
	$(call run_with_mocked_tx, test_ckbsyscalls.lua)
	$(call run, bn.lua)

ci: evaluate-lua-code-in-cell-data
	$(call run_ci, test_require.lua)
	$(call run_ci, test_loadfile.lua)
	$(call run_with_mocked_tx, test_ckbsyscalls.lua)
	$(call run, out_of_memory.lua) 2>&1 | fgrep 'not enough memory'
	$(call run, out_of_memory2.lua) 2>&1 | fgrep 'not enough memory'
	$(call run_ci, bn.lua)
	$(call run_ci, msgpack-tests.lua)

benchmark:
	$(call run, msgpack-benchmark.lua)
	
run-pretty:
	$(call run_pretty_result, test_$(TEST).lua)

run-debugger:
	$(CKB-DEBUGGER) --mode gdb --gdb-listen 127.0.0.1:${PORT} --read-file out_of_memory.lua --bin ../../build/lua-loader.debug -- lua-loader -r

