Note: the syscall spawn is a planned feature that is currently unavailable for use in ckb.
We need a new hard fork to ship the spawn syscall. We will update the table below
when the availablity of spawn syscall changes.

| network | readiness |
|---------|-----------|
| mainnet | NOT ready |
| testnet | NOT ready |

# Running ckb-lua as a subprocess to the main contract 
To demostrate how to extend the capabilities of a main contract with ckb-lua, we provide
an example (an admittedly contrived one) that spawn a ckb-lua subprocess which simply concatenate
the command line arguments and return the result to the main contract. 

[The main contract of this example](../examples/spawn.c) is written in c.
This program can be built with `make all-via-docker`. Afterwards, you may run it
with `make -C tests/test_cases spawnexample`.
The main contract first reserves some memory for the sub-contract, and then invokes ckb-lua with
the command line arguments `"-e" "local m = arg[2] .. arg[3]; ckb.set_content(m)" "hello" "world"`
which evaluates the lua code `local m = arg[2] .. arg[3]; ckb.set_content(m)`.

This Lua code will first concatenate the strings `hello` and `world`, and then return the result
to the main contract with `ckb.set_content`. The main contract may read the content on the subprocess exits.
