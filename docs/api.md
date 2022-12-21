ckb-lua API documentation.

# Exported Functions

## Partial Loading

The functions here may take a variable length of arguments. There is a [partial loading like mechanism](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#partial-loading) for Lua functions. Functions below with partial loading supported may be additionally passed to the argument length and offset. The behavior of these Lua functions is classified as follows

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

## Error Handling

Most of they may return an error as the last argument. The error is a non-zero integer that represents the kind of error that happened during the execution of the function. Callers should check errors to be nil before using other arguments.

## More Examples

[See ckb syscall test cases](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua).

## Functions

Note when partial loading support is enabled, the description for arguments length and offset is omitted. You may optionally pass length and offset in that case.


### `ckb.dump`
description: dump a returned buffer to stdout

calling example: [`ckb.dump(buf)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L101-L105)

arguments: buf (a buffer returned from syscalls below)

return values: none

side effects: the buf is printed

### `ckb.exit`
description: exit the ckb-vm execution

calling example: `ckb.exit(code)`

arguments: code (exit code)

return values: none

side effects: exit the ckb-vm execution

see also: [`ckb_exit` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#exit)

### `ckb.exit_script`
description: stop lua script execution, return to the ckb-vm caller

calling example: `ckb.exit_script(code)`

arguments: code (this will be the return code of `lua_run_code`, should be positive, see `lua_run_code` above for details)

return values: none

side effects: stop lua script execution, return to the ckb-vm caller

### `ckb.mount`
description: load the cell data and mount the file system witin in

calling example: `ckb.mount(source, index)`

arguments: source (the source of the cell to load), index (the index of the cell to load within all cells with source `source`)

return values: err (may be nil object to represent possible error)

side effects: the files within the file system will be available to use if no error happened

see also: [file system documentation](./fs.md)

### `ckb.load_tx_hash`
description: load the transaction hash

calling example: [`buf, err = ckb.load_tx_hash()`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L51-L53)

arguments: none

partial loading supported: yes

return values: buf (the buffer that contains the transaction hash), err (may be nil object to represent possible error)

see also: [`ckb_load_tx_hash` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-transaction-hash)

### `ckb.load_script_hash`
description: load the hash of current script

calling example: [`buf, error = ckb.load_script_hash()`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L63-L65)

arguments: none

partial loading supported: yes

return values: buf (the buffer that contains the script hash), err (may be nil object to represent possible error)

see also: [`ckb_load_script_hash` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-script-hash)

### `ckb.load_script`
description: load current script

calling example: [`buf, error = ckb.load_script()`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L59-L61)

arguments: none

partial loading supported: yes

return values: buf (the buffer that contains script), err (may be nil object to represent possible error)

see also: [`ckb_load_script` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-script)

### `ckb.load_and_unpack_script`
description: load and unpack current script

calling example: [`code_hash, hash_type, args, error = ckb.load_and_unpack_script()`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L172-L176)

arguments: none

return values: code_hash (the code hash of current script), hash_type (the hash type of current script), args (the args of current script), err (may be nil object to represent possible error)

### `ckb.load_transaction`
description: load current transaction

calling example: [`buf, error = ckb.load_transaction()`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L55-L57)

arguments: none

partial loading supported: yes

return values: buf (the buffer that contains current transaction), err (may be nil object to represent possible error)

see also: [`ckb_load_transaction` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-transaction)

### `ckb.load_cell`
description: load cell

calling example: [`buf, error = ckb.load_cell(index, source)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L67-L73)

arguments: index (the index of the cell), source (the source of the cell)

partial loading supported: yes

return values: buf (the buffer that contains cell), err (may be nil object to represent possible error)

see also: [`ckb_load_cell` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-cell)

### `ckb.load_input`
description: load input cell

calling example: [`buf, error = ckb.load_input(index, source)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L143-L145)

arguments: index (the index of the cell), source (the source of the cell)

partial loading supported: yes

return values: buf (the buffer that contains the input cell), err (may be nil object to represent possible error)

see also: [`ckb_load_input` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-input)

### `ckb.load_header`
description: load cell header

calling example: [`buf, error = ckb.load_header(index, source)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L160-L163)

arguments: index (the index of the cell), source (the source of the cell)

partial loading supported: yes

return values: buf (the buffer that contains the header), err (may be nil object to represent possible error)

see also: [`ckb_load_header` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-header)

### `ckb.load_witness`
description: load the witness

calling example: [`buf, error = ckb.load_witness(index, source, length, offset)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L3-L49)

arguments: index (the index of the cell), source (the source of the cell)

partial loading supported: yes

return values: buf (the buffer that contains the witness), err (may be nil object to represent possible error)

see also: [`ckb_load_witness` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-witness)

### `ckb.load_cell_data`
description: load cell data

calling example: [`buf, error = ckb.load_cell_data(index, source, length, offset)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L135-L141)

arguments: index (the index of the cell), source (the source of the cell)

partial loading supported: yes

return values: buf (the buffer that contains the cell data), err (may be nil object to represent possible error)

see also: [`ckb_load_cell_data` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-cell-data)

### `ckb.load_cell_by_field`
description: load cell data field

calling example: [`buf, error = ckb.load_cell_by_field(index, source, field)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L75-L133)

arguments: index (the index of the cell), source (the source of the cell), field (the field to load)

partial loading supported: yes

return values: buf (the buffer that contains the cell data field), err (may be nil object to represent possible error)

see also: [`ckb_load_cell_by_field` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-cell-by-field)

### `ckb.load_input_by_field`
description: load input field

calling example: [`buf, error = ckb.load_input_by_field(index, source, field)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L147-L155)

arguments: index (the index of the cell), source (the source of the cell), field (the field to load)

partial loading supported: yes

return values: buf (the buffer that contains the input field), err (may be nil object to represent possible error)

see also: [`ckb_load_input_by_field` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-input-by-field)

### `ckb.load_header_by_field`
description: load header by field

calling example: [`ckb.load_header_by_field(index, source, field)`](https://github.com/XuJiandong/ckb-lua/blob/4e5375eb2a866595f89826db5510bc9d1f8e9510/tests/test_cases/test_ckbsyscalls.lua#L157-L170)

arguments: index (the index of the cell), source (the source of the cell), field (the field to load)

partial loading supported: yes

return values: buf (the buffer that contains the header field), err (may be nil object to represent possible error)

see also: [`ckb_load_header_by_field` syscall](https://github.com/nervosnetwork/rfcs/blob/master/rfcs/0009-vm-syscalls/0009-vm-syscalls.md#load-header-by-field)

 Exported constants

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
