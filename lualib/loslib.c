/*
** $Id: loslib.c $
** Standard Operating System library
** See Copyright Notice in lua.h
*/

#define loslib_c
#define LUA_LIB

#include <stdlib.h>

#include "lauxlib.h"

static int os_exit(lua_State *L) {
    int status;
    if (lua_isboolean(L, 1))
        status = (lua_toboolean(L, 1) ? EXIT_SUCCESS : EXIT_FAILURE);
    else
        status = (int)luaL_optinteger(L, 1, EXIT_SUCCESS);
    if (lua_toboolean(L, 2)) lua_close(L);
    if (L) exit(status); /* 'if' to avoid warnings for unreachable 'return' */
    return 0;
}

static const luaL_Reg syslib[] = {{"exit", os_exit}, {NULL, NULL}};

LUAMOD_API int luaopen_os(lua_State *L) {
    luaL_newlib(L, syslib);
    return 1;
}
