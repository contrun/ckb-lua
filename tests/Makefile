

CKB-DEBUGGER ?= ckb-debugger

all:
	RUST_LOG=debug $(CKB-DEBUGGER) --read-file official/locals.lua  --bin ../build/lua-loader -- lua-loader -r
