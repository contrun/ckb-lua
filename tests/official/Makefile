CKB-DEBUGGER ?= ckb-debugger
MAX-CYCLES ?= 2000000000
TEST-FILE ?= 

define run
	RUST_LOG=debug $(CKB-DEBUGGER) --max-cycles $(MAX-CYCLES) --read-file $(1) --bin ../../build/lua-loader.debug -- -r   
endef

define run_ci
	RUST_LOG=debug $(CKB-DEBUGGER) --max-cycles $(MAX-CYCLES) --read-file $(1) --bin ../../build/lua-loader.debug -- -r  2>&1 | fgrep 'Run result: 0'
endef

define run_pprof
	RUST_LOG=debug $(CKB-DEBUGGER) --max-cycles $(MAX-CYCLES) --read-file $(1) --bin ../../build/lua-loader.debug --pprof $(1).pprof -- -r   
endef

all:
	$(call run, locals.lua)	
	$(call run, literals.lua)
	$(call run, sort.lua)
	$(call run, strings.lua)
	$(call run, math.lua)
	$(call run, api.lua)
	$(call run, bwcoercion.lua)
	$(call run, calls.lua)
	$(call run, closure.lua)
	$(call run, code.lua)
	$(call run, coroutine.lua)
	$(call run, events.lua)
	$(call run, gengc.lua)
	$(call run, goto.lua)
	$(call run, heavy.lua)
	$(call run, tpack.lua)
	$(call run, tracegc.lua)
	$(call run, utf8.lua)
	$(call run, vararg.lua)
	$(call run, gc.lua)
	$(call run, bitwise.lua)
	$(call run, constructs.lua)
	$(call run, db.lua)
	$(call run, errors.lua)
	$(call run, nextvar.lua)
	$(call run, pm.lua)
	$(call run, big.lua)

# failed tests:
# attrib runs some file operations
# cstack requires fwrite
# files has too many unsupported io operation
# main has too many unsupported io operation
# verybig creates a big lua script file, and then run it.

ci:
	$(call run_ci, locals.lua)
	$(call run_ci, literals.lua)
	$(call run_ci, sort.lua)
	$(call run_ci, strings.lua)
	# RUST_LOG=debug ckb-debugger --max-cycles 2000000000 --read-file  math.lua --bin ../../build/lua-loader.debug -- -e 'print(math.mininteger + 1.0)'
	# -2.3058430090000e+18
	# lua -e 'print(math.mininteger + 1.0)'
	# -9.2233720368548e+18
	$(call run_ci, math.lua)
	$(call run_ci, api.lua)
	$(call run_ci, bwcoercion.lua)
	$(call run_ci, calls.lua)
	$(call run_ci, closure.lua)
	$(call run_ci, code.lua)
	$(call run_ci, coroutine.lua)
	$(call run_ci, events.lua)
	$(call run_ci, gengc.lua)
	$(call run_ci, goto.lua)
	$(call run_ci, heavy.lua)
	$(call run_ci, tpack.lua)
	$(call run_ci, tracegc.lua)
	$(call run_ci, utf8.lua)
	$(call run_ci, vararg.lua)
	$(call run_ci, gc.lua)
	$(call run_ci, bitwise.lua)
	$(call run_ci, constructs.lua)
	$(call run_ci, db.lua)
	$(call run_ci, errors.lua)
	$(call run_ci, nextvar.lua)
	$(call run_ci, pm.lua)
	$(call run_ci, big.lua)

#
# Some test cases are removed. They will try to consume all the memory.
# On native machine, it can reach up to several GB but on ckb-vm it only has 4M.
# It's unfair.
#
perf-ckb-vm:
	$(call run_ci, literals.lua)
	$(call run_ci, sort.lua)
	$(call run_ci, strings.lua)
	$(call run_ci, math.lua)
	$(call run_ci, api.lua)
	$(call run_ci, bwcoercion.lua)
	$(call run_ci, calls.lua)
	$(call run_ci, closure.lua)
	$(call run_ci, code.lua)
	$(call run_ci, coroutine.lua)
	$(call run_ci, events.lua)
	$(call run_ci, gengc.lua)
	$(call run_ci, goto.lua)
	$(call run_ci, tpack.lua)
	$(call run_ci, tracegc.lua)
	$(call run_ci, utf8.lua)
	$(call run_ci, vararg.lua)
	$(call run_ci, gc.lua)
	$(call run_ci, bitwise.lua)
	$(call run_ci, constructs.lua)
	$(call run_ci, db.lua)
	$(call run_ci, nextvar.lua)
	$(call run_ci, pm.lua)
	$(call run_ci, big.lua)

perf-native:
	lua literals.lua
	lua sort.lua
	lua strings.lua
	lua math.lua
	lua api.lua
	lua bwcoercion.lua
	lua calls.lua
	lua closure.lua
	lua code.lua
	lua coroutine.lua
	lua events.lua
	lua gengc.lua
	lua goto.lua
	lua tpack.lua
	lua tracegc.lua
	lua utf8.lua
	lua vararg.lua
	lua gc.lua
	lua bitwise.lua
	lua constructs.lua
	lua db.lua
	lua nextvar.lua
	lua pm.lua
	lua big.lua

compile:
	luac -s -o sort.bc sort.lua

pprof:
	$(call run_pprof, sort.bc)

test-file:
	$(call run, $(TEST-FILE).lua)	
