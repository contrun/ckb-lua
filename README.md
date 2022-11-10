# ckb-lua
A project to help developers writing script in Lua on [CKB-VM](https://github.com/nervosnetwork/ckb-vm).


## Build

```bash
make all-via-docker
```


## Usages

There are 2 ways to use this project:
1. Embedded

Use `build/libckblua.so` as a shared library. See [dylib.md](./docs/dylib.md) for a detailed tutorial on how to use `build/libckblua.so`.

2. Standalone(used as script)

Use `build/lua-loader` as a script.
