

CKB-DEBUGGER ?= ckb-debugger

define run
	RUST_LOG=debug $(CKB-DEBUGGER) --read-file $(1)  --bin ../../build/lua-loader.debug -- lua-loader -r   
endef

define run_ci
	RUST_LOG=debug $(CKB-DEBUGGER) --read-file $(1)  --bin ../../build/lua-loader.debug -- lua-loader -r  2>&1 | fgrep 'Run result: 0'
endef


all:
	$(call run, locals.lua)	
	$(call run, literals.lua)
	$(call run, sort.lua)
	$(call run, strings.lua)

ci:
	$(call run_ci, locals.lua)
	$(call run_ci, literals.lua)
	$(call run_ci, sort.lua)
	$(call run_ci, strings.lua)
