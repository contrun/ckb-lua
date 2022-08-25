

CKB-DEBUGGER ?= ckb-debugger
PORT ?= 9999

define run
	RUST_LOG=debug $(CKB-DEBUGGER) --read-file $(1)  --bin ../../build/lua-loader.debug -- lua-loader -r   
endef

define run_ci
	RUST_LOG=debug $(CKB-DEBUGGER) --read-file $(1)  --bin ../../build/lua-loader.debug -- lua-loader -r  2>&1 | fgrep 'Run result: 0'
endef

all:
	$(call run, test_require.lua)
	$(call run, test_loadfile.lua)

ci:
	$(call run_ci, test_require.lua)
	$(call run_ci, test_loadfile.lua)
	$(call run, out_of_memory.lua) 2>&1 | fgrep 'not enough memory'

run-debugger:
	$(CKB-DEBUGGER) --mode gdb --gdb-listen 127.0.0.1:${PORT} --read-file out_of_memory.lua --bin ../../build/lua-loader.debug -- lua-loader -r

