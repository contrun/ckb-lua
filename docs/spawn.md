Note: the syscall spawn is a planned feature that is currently unavailable for use in ckb.
We need a new hard fork to ship the spawn syscall. We will update the table below
when the availablity of spawn syscall changes.

| network | readiness |
|---------|-----------|
| mainnet | NOT ready |
| testnet | NOT ready |

# Running ckb-lua as a subprocess to the main script
There are quite a few benefits in running ckb-lua as a subprocess to the main script.
- The main script's context is saved, and can continue to run when ckb-lua exits.
- The ckb-lua instances are called with command line arguments which can be used to differentiate different tasks.
- Ckb-lua may return data of any format by the function `ckb.set_content`.

To demostrate how to extend the capabilities of a main script with ckb-lua, we provide
an example (an admittedly contrived one) that spawn a ckb-lua subprocess which simply concatenate
the command line arguments and return the result to the main script. 

[The main script of this example](../examples/spawn.c) is written in c.
This program can be built with `make all-via-docker`. Afterwards, you may run it
with `make -C tests/test_cases spawnexample` which requries [this branch of ckb-standalone-debugger](https://github.com/mohanson/ckb-standalone-debugger/tree/syscall_spawn).

The main script first reserves some memory for the sub-contract, and then invokes ckb-lua with
the command line arguments `"-e" "local m = arg[2] .. arg[3]; ckb.set_content(m)" "hello" "world"`
which evaluates the lua code `local m = arg[2] .. arg[3]; ckb.set_content(m)`.

This Lua code will first concatenate the strings `hello` and `world`, and then return the result
to the main script with `ckb.set_content`. The main script may read the content on the subprocess exits.
