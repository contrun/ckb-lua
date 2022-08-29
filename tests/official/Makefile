CKB-DEBUGGER ?= ckb-debugger
MAX-CYCLES ?= 2000000000
TEST-FILE ?= 

define run
	RUST_LOG=debug $(CKB-DEBUGGER) --max-cycles $(MAX-CYCLES) --read-file $(1) --bin ../../build/lua-loader.debug -- lua-loader -r   
endef

define run_ci
	RUST_LOG=debug $(CKB-DEBUGGER) --max-cycles $(MAX-CYCLES) ---read-file $(1) --bin ../../build/lua-loader.debug -- lua-loader -r  2>&1 | fgrep 'Run result: 0'
endef

all:
	$(call run, locals.lua)	
	$(call run, literals.lua)
	$(call run, sort.lua)
	$(call run, strings.lua)
	$(call run, math.lua)

ci:
	$(call run_ci, locals.lua)
	$(call run_ci, literals.lua)
	$(call run_ci, sort.lua)
	$(call run_ci, strings.lua)
	$(call run_ci, math.lua)

test-file:
	$(call run, $(TEST-FILE).lua)	
