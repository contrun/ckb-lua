#ifndef LUA_CKB_H
#define LUA_CKB_H

#include "lua.h"

#define LUA_ERROR_INTERNAL 100
#define LUA_ERROR_OUT_OF_MEMORY 101
#define LUA_ERROR_ENCODING 102
#define LUA_ERROR_SCRIPT_TOO_LONG 103
#define LUA_ERROR_INVALID_ARGUMENT 104
#define LUA_ERROR_INVALID_STATE 105
#define LUA_ERROR_SYSCALL 106
#define LUA_ERROR_NOT_IMPLEMENTED 107

const char *CKB_RETURN_CODE_KEY = "_ckb_return_code";

static int s_lua_exit_enabled = 0;
#endif
