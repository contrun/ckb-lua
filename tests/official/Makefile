

CKB-DEBUGGER ?= ckb-debugger

define run
	RUST_LOG=debug $(CKB-DEBUGGER) --read-file $(1)  --bin ../../build/lua-loader.debug -- lua-loader -r   
endef

all:
	$(call run, locals.lua)	
	$(call run, literals.lua)
	$(call run, sort.lua)
	$(call run, strings.lua)

string:
	$(call run, strings.lua)
