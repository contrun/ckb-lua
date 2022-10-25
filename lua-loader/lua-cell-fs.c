#include "../include/ckb_cell_fs.c"
#include "lua-cell-fs-test.c"

static int run_from_file_system(lua_State *L) {
    return run_file_system_tests(L);
}
