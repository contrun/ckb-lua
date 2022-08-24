

CKB-DEBUGGER ?= ckb-debugger
PORT ?= 9999

define run
	RUST_LOG=debug $(CKB-DEBUGGER) --read-file $(1)  --bin ../../build/lua-loader.debug -- lua-loader -r   
endef

all:
	$(call run, test_require.lua)
	$(call run, test_loadfile.lua)

oom:
	$(call run, out_of_memory.lua)

run-debugger:
	$(CKB-DEBUGGER) --mode gdb --gdb-listen 127.0.0.1:${PORT} --read-file out_of_memory.lua --bin ../../build/lua-loader.debug -- lua-loader -r

