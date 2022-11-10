# Calling Lua code from your ckb contracts

To facilitate the development of contracts on ckb platform, we recently ported Lua, a scripting language famous for its simplicity and ease of use, to the ckb-vm. Ckb developers can now both run the standalone Lua interpreter in the ckb platform and embed Lua code into their main ckb contracts. This greatly lower the barrier to entry for ckb contracts development.

[dylib.c](https://github.com/XuJiandong/ckb-lua/blob/70bc3e7bff3521a600612e52b9a2aee56c3efca2/examples/dylib.c) a sample program that evaluates lua code with ckb-lua shared library. We showcase how to load and unpack script args by a few lines of Lua code, i.e.

```lua
local _code_hash, _hash_type, args, err = ckb.load_and_unpack_script()
print(err)
if err == nil then
	ckb.dump(args)
end
```

Of course, this example is not enough for real world usage, but this article should be enough to show how to use ckb-lua shared library from the main contract program. You can unleash the potential of Lua with your creativity afterward. For the curious, see here for [an implementation of sudt in lua](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/contracts/sudt.lua). Also note that, the sample code is written in c. As an alternative, you can write test code in rust. Both [loading shared library](https://docs.rs/ckb-std/latest/ckb_std/dynamic_loading/index.html) and mocking transaction to do unit tests are easy in rust. 

I will now explain what this sample code does and how to run it.

# Building Shared Library

The sample code depends on `libckblua.so` shared library to run Lua code. We need to obtain the source of ckb-lua from [the github repository](https://github.com/XuJiandong/ckb-lua), and build it ourselves. `make all-via-docker` is enough to build all the binaries. `./build/libckblua.so` may be used afterward. The sample program `./build/dylibexample` built from [dylib.c](https://github.com/XuJiandong/ckb-lua/blob/70bc3e7bff3521a600612e52b9a2aee56c3efca2/examples/dylib.c) is also available to use.

# Preparing Shared Library

## Packing the Shared Library Into the Dependent Cell Data

To use the ckb-lua shared library, we need to store the shared library in some cell data and specify the cell with shared library data as a dependent cell. We have assembled a [mock transaction file](https://crates.io/crates/ckb-mock-tx-types) [here](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/dylib.json). You can also mock this with the ckb rust SDK (like [this](https://github.com/nervosnetwork/ckb-production-scripts/blob/master/tests/xudt_rce_rust/tests/test_xudt_rce.rs)). You can use it along with [ckb-standalone-debugger](https://github.com/nervosnetwork/ckb-standalone-debugger) to run the sample code. For example, you can run `./build/dylibexample` with `ckb-debugger --tx-file ./tests/test_cases/dylib.json --script-group-type=type --cell-index=0 --cell-type=output --bin build/dylibexample`.

## Passing the Dependent Cell Information with Script Args

We need to pass the cell information to the args of the main c contract.

We performed some standard operations to obtain the args of the current script, i.e. loading the script with `ckb_load_script` and then de-serialize the args with molecule.

After we have obtained the args, we can unpack the cell information contained within the args.

In our example, our args has the following format.

```
// <reserved args, 2 bytes> <code hash of the shared library, 32 bytes>
// <hash type of shared library, 1 byte>
```

Given the size and offset of fields, we can easily obtain the code hash and hash type of the dependent cell. Now, we can open the shared library by `ckb_dlopen2`.

## Opening the Shared Library and Calling the Functions

[`ckb_dlopen2`](https://docs.nervos.org/docs/essays/dependencies/#dynamic-linking)

```c
    err = ckb_dlopen2(code_hash, hash_type, code_buff, code_buff_size, handle, &consumed_size);
```

is a [ckb-c-stdlib](https://github.com/nervosnetwork/ckb-c-stdlib) function to load the shared library from dependent cell(s) with specific `code_hash` and `hash_type`. If the shared library is found, `ckb_dlopen2` will then write the shared library handle to the `handle` parameter. The `handle` parameter can then be used to obtain pointers to our Lua functions.

# Calling Lua Script

We can use the following exported functions to call Lua scripts.

## Exported c Functions in the Shared Library

Three functions have been exported in the shared library.

1. `void *lua_create_instance(uintptr_t min, uintptr_t max)`
2. `int lua_run_code(void *l, const char *code, size_t code_size, char *name)`
3. `void close_lua_instance(void *L)`

### `lua_create_instance`

To create new Lua instance, we may use `lua_create_instance`. In order to avoid the collision of lua’s malloc and the host program’s malloc function. We need to specify the upper and lower bound of the memory that can be used exclusively by Lua (this is a contract that should be enforced by the host program). The host program can allocate this space in the stack or heap. The return value of this function is an opaque pointer that can be used to evaluate Lua code with `lua_run_code`. We can reclaim the resources by calling `lua_close_instance`.

### `lua_run_code`

The actual evaluation of Lua code can be initiated by calling`lua_run_code`. Both Lua source code and binary code are acceptable to `lua_run_code`. Evaluating binary code can be less resource-hungry both in terms of the cycles needed to run the code and the storage space required to store the code. It takes four arguments. The first one `l` is the opaque pointer returned from `lua_create_instance`, the second one `code` is a pointer which points to the code buffer, which can be either Lua source code or Lua bytecode. The third one `code_size` is the size of the `code` buffer. The last one `name` is the name of the Lua program. This may be helpful in debugging.

### `lua_close_instance`

Calling `lua_close_instance` will release all the resources used by Lua. It takes the opaque pointer returned from `lua_create_instance` as a parameter.

## Lua Functions

### Functions in Lua Standard Library

Most of the functions in Lua standard library have been ported to ckb-lua. You can expect all the platform-independent functions to work. But some platform-dependent functions like `os.rename` (removing a file from the file system) will not exist in ckb-lua.

TODO: add list of functions that are not available in ckb-lua.

### CKB Specific Functions

On the other hand, we have added a few ckb-specific helper functions to interact with ckb-vm. These include functions for issuing syscalls, debugging, and unpacking data structures. All of them are located in the `ckb` Lua module.

See the appendix for a list of exported constants and functions.

## Stitching Things Together

Actually running the lua code is quite easy.

```c
    CreateLuaInstanceFuncType create_func =
        must_load_function(handle, "lua_create_instance");
    EvaluateLuaCodeFuncType evaluate_func =
        must_load_function(handle, "lua_run_code");
    CloseLuaInstanceFuncType close_func =
        must_load_function(handle, "lua_close_instance");

    const size_t mem_size = 1024 * 512;
    uint8_t mem[mem_size];

    void* l = create_func((uintptr_t)mem, (uintptr_t)(mem + mem_size));
    if (l == NULL) {
        printf("creating lua instance failed\n");
        return;
    }

    int ret = evaluate_func(l, code, code_size, "test");

    if (ret != 0) {
        printf("evaluating lua code failed: %d\n", ret);
        return;
    }
    close_func(l);
```

We first load the functions from the previous returned dynamic library handle, and then create a stack space for Lua to use. Afterwards, we can call the exported functions.

The Lua code

```lua
local _code_hash, _hash_type, args, err = ckb.load_and_unpack_script()
print(err)
if err == nil then
	ckb.dump(args)
end
```

loads the ckb script and unpacks it into `code_hash`, `hash_type` and `args`. If there is anything wrong when performing this operation, an error would return in `err`. Since the script args can be arbitrary binary data, it may be hard to inspect. We have also added a `dump` function to dump the hex and ASCII representations of the data.

# Request for Comments

The prototype of ckb-lua is now somewhat complete. We are now gathering feedback on the usage of ckb-lua. If you have any use cases that are not easily met by the current ckb-lua implementation in mind, please feel free to leave comments at the [ckb-lua](https://github.com/XuJiandong/ckb-lua) repository.

# Appendix: Exported Lua Functions and Constants in the CKB Module

## Exported Functions

### Partial Loading

The functions here may take a variable length of arguments. There is a [partial loading like mechanism](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#partial-loading) for Lua functions. Functions below with partial loading supported may be additionally passed to the argument length and offset. The behavior of these Lua functions depends is classified as follows

- Both length and offset are omitted.
In this case, the whole data would be loaded, e.g. calling `ckb.load_witness(0, ckb.SOURCE_INPUT)` would load the whole witness of input cell of index 0.
- The offset is omitted, and the length passed is zero.
In this case, the length of the whole data would be returned, e.g. `ckb.load_witness(0, ckb.SOURCE_INPUT, 0)` will return the length of the witness instead of the witness data.
- The offset is omitted, and the length passed is non-zero.
In this case, the initial data of the argument length bytes would be returned, e.g. `ckb.load_witness(0, ckb.SOURCE_INPUT, 10)` will return the initial 10 bytes of witness.
- Both length and offset are passed, and the length passed is zero.
In this case, the data length starting from offset would be returned, e.g. calling `ckb.load_witness(0, ckb.SOURCE_INPUT, 0, 10)` would the data length starting from offset 10.
- Both length and offset are passed, and the length passed is non-zero.
In this case, the data starting from offset of length would be returned, e.g. calling `ckb.load_witness(0, ckb.SOURCE_INPUT, 2, 10)` would return 2 bytes of data starting from 10.

### Error Handling

Most of them may return an error as the last argument. The error is a non-zero integer that represents the kind of error that happened during the execution of the function. Callers should check errors to be nil before using other arguments.

### More Examples

[See ckb syscall test cases](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua).

### Functions

Note when partial loading support is enabled, the description for arguments length and offset is omitted. You may optionally pass length and offset in that case.


#### `ckb.dump`
description: dump a returned buffer to stdout

calling example: [`ckb.dump(buf)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L101-L105)

arguments: buf (a buffer returned from syscalls below)

return values: none

side effects: the buf is printed

#### `ckb.exit`
description: exit the ckb-vm execution

calling example: `ckb.exit(code)`

arguments: code (return code)

return values: none

side effects: exit the ckb-vm execution

see also: [`ckb_exit` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#exit)

#### `ckb.load_tx_hash`
description: load the transaction hash

calling example: [`buf, err = ckb.load_tx_hash()`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L51-L53)

arguments: none

partial loading supported: yes

return values: buf (the buffer that contains the transaction hash), err (may be nil object to represent possible error)

see also: [`ckb_load_tx_hash` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-transaction-hash)

#### `ckb.load_script_hash`
description: load the hash of current script

calling example: [`buf, error = ckb.load_script_hash()`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L63-L65)

arguments: none

partial loading supported: yes

return values: buf (the buffer that contains the script hash), err (may be nil object to represent possible error)

see also: [`ckb_load_script_hash` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-script-hash)

#### `ckb.load_script`
description: load current script

calling example: [`buf, error = ckb.load_script()`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L59-L61)

arguments: none

partial loading supported: yes

return values: buf (the buffer that contains script), err (may be nil object to represent possible error)

see also: [`ckb_load_script` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-script)

#### `ckb.load_and_unpack_script`
description: load and unpack current script

calling example: [`code_hash, hash_type, args, error = ckb.load_and_unpack_script()`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L172-L176)

arguments: none

return values: code_hash (the code hash of current script), hash_type (the hash type of current script), args (the args of current script), err (may be nil object to represent possible error)

#### `ckb.load_transaction`
description: load current transaction

calling example: [`buf, error = ckb.load_transaction()`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L55-L57)

arguments: none

partial loading supported: yes

return values: buf (the buffer that contains current transaction), err (may be nil object to represent possible error)

see also: [`ckb_load_transaction` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-transaction)

#### `ckb.load_cell`
description: load cell

calling example: [`buf, error = ckb.load_cell(index, source)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L67-L73)

arguments: index (the index of the cell), source (the source of the cell)

partial loading supported: yes

return values: buf (the buffer that contains cell), err (may be nil object to represent possible error)

see also: [`ckb_load_cell` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-cell)

#### `ckb.load_input`
description: load input cell

calling example: [`buf, error = ckb.load_input(index, source)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L143-L145)

arguments: index (the index of the cell), source (the source of the cell)

partial loading supported: yes

return values: buf (the buffer that contains the input cell), err (may be nil object to represent possible error)

see also: [`ckb_load_input` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-input)

#### `ckb.load_header`
description: load cell header

calling example: [`buf, error = ckb.load_header(index, source)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L160-L163)

arguments: index (the index of the cell), source (the source of the cell)

partial loading supported: yes

return values: buf (the buffer that contains the header), err (may be nil object to represent possible error)

see also: [`ckb_load_header` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-header)

#### `ckb.load_witness`
description: load the witness

calling example: [`buf, error = ckb.load_witness(index, source, length, offset)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L3-L49)

arguments: index (the index of the cell), source (the source of the cell)

partial loading supported: yes

return values: buf (the buffer that contains the witness), err (may be nil object to represent possible error)

see also: [`ckb_load_witness` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-witness)

#### `ckb.load_cell_data`
description: load cell data

calling example: [`buf, error = ckb.load_cell_data(index, source, length, offset)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L135-L141)

arguments: index (the index of the cell), source (the source of the cell)

partial loading supported: yes

return values: buf (the buffer that contains the cell data), err (may be nil object to represent possible error)

see also: [`ckb_load_cell_data` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-cell-data)

#### `ckb.load_cell_by_field`
description: load cell data field

calling example: [`buf, error = ckb.load_cell_by_field(index, source, field)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L75-L133)

arguments: index (the index of the cell), source (the source of the cell), field (the field to load)

partial loading supported: yes

return values: buf (the buffer that contains the cell data field), err (may be nil object to represent possible error)

see also: [`ckb_load_cell_by_field` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-cell-by-field)

#### `ckb.load_input_by_field`
description: load input field

calling example: [`buf, error = ckb.load_input_by_field(index, source, field)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L147-L155)

arguments: index (the index of the cell), source (the source of the cell), field (the field to load)

partial loading supported: yes

return values: buf (the buffer that contains the input field), err (may be nil object to represent possible error)

see also: [`ckb_load_input_by_field` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-input-by-field)

#### `ckb.load_header_by_field`
description: load header by field

calling example: [`ckb.load_header_by_field(index, source, field)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L157-L170)

arguments: index (the index of the cell), source (the source of the cell), field (the field to load)

partial loading supported: yes

return values: buf (the buffer that contains the header field), err (may be nil object to represent possible error)

see also: [`ckb_load_header_by_field` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-header-by-field)

## Exported constants

While most constants here are directly taken from [ckb_consts.h](https://github.com/nervosnetwork/ckb-system-scripts/blob/master/c/ckb_consts.h), we also defined some are ckb-lua specific constants like `ckb.LUA_ERROR_INTERNAL`.

```
ckb.SUCCESS
ckb.INDEX_OUT_OF_BOUND
ckb.ITEM_MISSING
ckb.LENGTH_NOT_ENOUGH
ckb.INVALID_DATA
ckb.LUA_ERROR_INTERNAL
ckb.LUA_ERROR_OUT_OF_MEMORY
ckb.LUA_ERROR_ENCODING
ckb.LUA_ERROR_SCRIPT_TOO_LONG
ckb.LUA_ERROR_INVALID_ARGUMENT

ckb.SOURCE_INPUT
ckb.SOURCE_OUTPUT
ckb.SOURCE_CELL_DEP
ckb.SOURCE_HEADER_DEP
ckb.SOURCE_GROUP_INPUT
ckb.SOURCE_GROUP_OUTPUT

ckb.CELL_FIELD_CAPACITY
ckb.CELL_FIELD_DATA_HASH
ckb.CELL_FIELD_LOCK
ckb.CELL_FIELD_LOCK_HASH
ckb.CELL_FIELD_TYPE
ckb.CELL_FIELD_TYPE_HASH
ckb.CELL_FIELD_OCCUPIED_CAPACITY

ckb.INPUT_FIELD_OUT_POINT
ckb.INPUT_FIELD_SINCE

ckb.HEADER_FIELD_EPOCH_NUMBER
ckb.HEADER_FIELD_EPOCH_START_BLOCK_NUMBER
ckb.HEADER_FIELD_EPOCH_LENGTH
```
