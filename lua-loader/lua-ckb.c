#include "ckb_dlfcn.h"
#include "ckb_syscalls.h"
#include "lauxlib.h"
#include "lualib.h"
#include <stdlib.h>
#include <stdarg.h>

typedef const char *string;
typedef int syscall_v2(void *, uint64_t *, size_t);
typedef int syscall_v4(void *, uint64_t *, size_t, size_t, size_t);
typedef int syscall_v5(void *, uint64_t *, size_t, size_t, size_t, size_t);

typedef enum {
    STRING = 1 << 0,
    UINT64 = 1 << 1,
    SIZE_T = 1 << 2,
    INTEGER = 1 << 3,
    BUFFER = 1 << 4,
} FIELD_TYPE;

typedef struct {
    void *buffer;
    size_t length;
} BUFFER_T;

typedef union {
    string str;
    uint64_t u64;
    size_t size;
    int integer;
    BUFFER_T buffer;
} FIELD_ARG;

typedef struct {
    string name;
    FIELD_TYPE type;
    FIELD_ARG arg;
} FIELD;

struct syscall_result_t {
    void *buffer;
    uint64_t length;
};

union syscall_function_union {
    syscall_v2 *v2f;
    syscall_v4 *v4f;
    syscall_v5 *v5f;
};

struct syscall_function_t {
    int discriminator;
    union syscall_function_union function;
};

#define CKB_LUA_OUT_OF_MEMORY 101
#define LUA_ERROR_ENCODING 102
#define LUA_ERROR_SCRIPT_TOO_LONG 103
#define LUA_ERROR_ARGUMENTS_LEN 104

/////////////////////////////////////////////////////
// Utilities
/////////////////////////////////////////////////////

#define THROW_ERROR(L, s, ...)                             \
    char _error[256];                                      \
    snprintf_(_error, sizeof(_error) - 1, s, __VA_ARGS__); \
    lua_pushstring(L, _error);                             \
    lua_error(L);

int call_syscall(struct syscall_function_t *f, uint8_t *buf, uint64_t *length,
                 ...) {
    size_t offset, index, source, field;
    va_list va_args;
    va_start(va_args, length);
    switch (f->discriminator) {
        case 2:
            offset = va_arg(va_args, size_t);
            va_end(va_args);
            return f->function.v2f(buf, length, offset);
        case 4:
            offset = va_arg(va_args, size_t);
            index = va_arg(va_args, size_t);
            source = va_arg(va_args, size_t);
            va_end(va_args);
            return f->function.v4f(buf, length, offset, index, source);
        case 5:
            offset = va_arg(va_args, size_t);
            index = va_arg(va_args, size_t);
            source = va_arg(va_args, size_t);
            field = va_arg(va_args, size_t);
            va_end(va_args);
            return f->function.v5f(buf, length, offset, index, source, field);
        default:
            va_end(va_args);
    }
    printf("invalid discriminator %d", f->discriminator);
    ckb_exit(-1);
    return -1;
}

int call_syscall_get_result(struct syscall_result_t *result,
                            struct syscall_function_t *f, uint64_t *length, ...) {
    int ret = 0;
    va_list va_args;
    va_start(va_args, length);
    /* Only obtain the minimal buffer length required */
    if (length != NULL && *length == 0) {
        ret = call_syscall(f, NULL, length, va_args);
        if (ret == 0) {
            result->length = *length;
        }
        goto exit;
    }

    size_t buflen = 0;
    if (length == NULL) {
        ret = call_syscall(f, NULL, &buflen, va_args);
        if (ret != 0) {
            goto exit;
        }
    } else {
        buflen = *length;
    }
    uint8_t *buf = malloc(buflen);
    if (buf == NULL) {
        ret = CKB_LUA_OUT_OF_MEMORY;
        goto exit;
    }

    ret = call_syscall(f, buf, length, va_args);
    if (ret != 0) {
        free(buf);
        goto exit;
    }

    /* We have passed a buffer with a size larger than what is
     * needed */
    if (length != NULL && *length < buflen) {
        buflen = *length;
    }
    result->buffer = buf;
    result->length = buflen;
exit:
    va_end(va_args);
    return ret;
}

int call_syscall_push_result(lua_State *L, struct syscall_function_t *f,
                             uint64_t *length, ...) {
    struct syscall_result_t result = {.buffer = NULL, .length = 0};
    va_list va_args;
    va_start(va_args, length);
    int ret = call_syscall_get_result(&result, f, length, va_args);
    va_end(va_args);
    if (ret != 0) {
        lua_pushnil(L);
        lua_pushinteger(L, ret);
        return 2;
    }
    if (result.buffer == NULL) {
        lua_pushinteger(L, result.length);
        lua_pushnil(L);
        return 2;
    }
    lua_pushlstring(L, (char *)result.buffer, result.length);
    free(result.buffer);
    lua_pushnil(L);
    return 2;
}

int call_syscall_push_result_v2(lua_State *L, syscall_v2 *f, uint64_t *length,
                                ...) {
    struct syscall_function_t nf = {.discriminator = 2, .function.v2f = f};
    va_list va_args;
    va_start(va_args, length);
    int ret = call_syscall_push_result(L, &nf, length, va_args);
    va_end(va_args);
    return ret;
}

int call_syscall_push_result_v4(lua_State *L, syscall_v4 *f, uint64_t *length,
                                ...) {
    struct syscall_function_t nf = {.discriminator = 4, .function.v4f = f};
    va_list va_args;
    va_start(va_args, length);
    int ret = call_syscall_push_result(L, &nf, length, va_args);
    va_end(va_args);
    return ret;
}

int call_syscall_push_result_v5(lua_State *L, syscall_v5 *f, uint64_t *length,
                                ...) {
    struct syscall_function_t nf = {.discriminator = 5, .function.v5f = f};
    va_list va_args;
    va_start(va_args, length);
    int ret = call_syscall_push_result(L, &nf, length, va_args);
    va_end(va_args);
    return ret;
}

#define SET_FIELD(L, v, n) \
    lua_pushinteger(L, v); \
    lua_setfield(L, -2, n);

int GET_FIELDS_WITH_CHECK(lua_State *L, FIELD *fields, int count,
                          int minimal_count) {
    int args_count = lua_gettop(L);
    if (args_count < minimal_count) {
        THROW_ERROR(L, "Invalid arguements count: expected %d got %d",
                    minimal_count, args_count)
    }
    for (int i = 0; i < count; ++i) {
        FIELD *field = &fields[i];

        switch (field->type) {
            case STRING: {
                if (i < minimal_count && lua_isstring(L, i + 1) == 0) {
                    THROW_ERROR(L,
                                "Invalid arguement \"%s\" at %d: need a string",
                                field->name, i + 1)
                }
                field->arg.str = lua_tostring(L, i + 1);
            } break;

            case UINT64: {
                if (i < minimal_count && lua_isinteger(L, i + 1) == 0) {
                    THROW_ERROR(
                        L, "Invalid arguement \"%s\" at %d: need an integer",
                        field->name, i + 1)
                }
                field->arg.u64 = lua_tointeger(L, i + 1);
            } break;

            case SIZE_T: {
                if (i < minimal_count && lua_isinteger(L, i + 1) == 0) {
                    THROW_ERROR(
                        L, "Invalid arguement \"%s\" at %d: need an integer",
                        field->name, i + 1)
                }
                field->arg.size = lua_tointeger(L, i + 1);
            } break;

            case INTEGER: {
                if (i < minimal_count && lua_isinteger(L, i + 1) == 0) {
                    THROW_ERROR(
                        L, "Invalid arguement \"%s\" at %d: need an integer",
                        field->name, i + 1)
                }
                field->arg.integer = lua_tointeger(L, i + 1);
            } break;

            case BUFFER: {
                if (i < minimal_count && lua_isstring(L, i + 1) == 0) {
                    THROW_ERROR(L,
                                "Invalid arguement \"%s\" at %d: need a string",
                                field->name, i + 1)
                }
                size_t length = 0;
                void *buffer = (void *)lua_tolstring(L, i + 1, &length);
                BUFFER_T b = {buffer, length};
                field->arg.buffer = b;
            } break;
        }
    }
    return args_count;
}

void setLengthAndOffset(FIELD *fields, int fields_count, uint64_t **length,
                        size_t *offset) {
    switch (fields_count) {
        case 0:
            break;

        case 1:
            *length = &fields[0].arg.u64;

        case 2:
            *length = &fields[0].arg.u64;
            *offset = fields[1].arg.size;
            break;
    }
}

int CKB_LOAD_V2(lua_State *L, syscall_v2 f) {
    FIELD fields[] = {{"length?", UINT64}, {"offset?", SIZE_T}};

    uint64_t *length = NULL;
    size_t offset = 0;
    setLengthAndOffset(fields, GET_FIELDS_WITH_CHECK(L, fields, 2, 0), &length,
                       &offset);

    return call_syscall_push_result_v2(L, f, length, offset);
}

int CKB_LOAD_V4(lua_State *L, syscall_v4 f) {
    FIELD fields[] = {
        {"index", SIZE_T},
        {"source", SIZE_T},
        {"length?", UINT64},
        {"offset?", SIZE_T},
    };

    uint64_t *length = NULL;
    size_t offset = 0;
    setLengthAndOffset(fields + 2, GET_FIELDS_WITH_CHECK(L, fields, 4, 2) - 2,
                       &length, &offset);

    return call_syscall_push_result_v4(L, f, length, offset, fields[0].arg.size,
                                       fields[1].arg.size);
}

int CKB_LOAD_V5(lua_State *L, syscall_v5 f) {
    FIELD fields[] = {
        {"index", SIZE_T},   {"source", SIZE_T},  {"field", SIZE_T},
        {"length?", UINT64}, {"offset?", SIZE_T},
    };

    uint64_t *length = NULL;
    size_t offset = 0;
    setLengthAndOffset(fields + 3, GET_FIELDS_WITH_CHECK(L, fields, 5, 3) - 3,
                       &length, &offset);

    return call_syscall_push_result_v5(L, f, length, offset, fields[0].arg.size,
                                       fields[1].arg.size, fields[2].arg.size);
}

// Usage:
//     hexDump(desc, addr, len, perLine);
//         desc:    if non-NULL, printed as a description before hex dump.
//         addr:    the address to start dumping from.
//         len:     the number of bytes to dump.
//         perLine: number of bytes on each output line.

void hexDump(const char *desc, const void *addr, const int len, int perLine) {
    // Silently ignore silly per-line values.

    if (perLine < 4 || perLine > 64) perLine = 16;

    int i;
    unsigned char buff[perLine + 1];
    const unsigned char *pc = (const unsigned char *)addr;

    // Output description if given.

    if (desc != NULL) printf("%s:\n", desc);

    // Length checks.

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %d\n", len);
        return;
    }

    // Process every byte in the data.

    for (i = 0; i < len; i++) {
        // Multiple of perLine means new or first line (with line offset).

        if ((i % perLine) == 0) {
            // Only print previous-line ASCII buffer for lines beyond first.

            if (i != 0) printf("  %s\n", buff);

            // Output the offset of current line.

            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.

        printf(" %02x", pc[i]);

        // And buffer a printable ASCII character for later.

        if ((pc[i] < 0x20) || (pc[i] > 0x7e))  // isprint() may be better.
            buff[i % perLine] = '.';
        else
            buff[i % perLine] = pc[i];
        buff[(i % perLine) + 1] = '\0';
    }

    // Pad out last line if not exactly perLine characters.

    while ((i % perLine) != 0) {
        printf("   ");
        i++;
    }

    // And print the final ASCII buffer.

    printf("  %s\n", buff);
}

int lua_ckb_dump(lua_State *L) {
    FIELD fields[] = {
        {"buffer", BUFFER},
        {"description?", STRING},
        {"perline?", INTEGER},
    };
    int args_count = GET_FIELDS_WITH_CHECK(L, fields, 3, 1);
    string desc = (args_count >= 2) ? fields[1].arg.str : "";
    int perline = (args_count >= 3) ? fields[2].arg.integer : 0;
    hexDump(desc, fields[0].arg.buffer.buffer, fields[0].arg.buffer.length,
            perline);
    return 0;
}

int lua_ckb_exit(lua_State *L) {
    FIELD fields[] = {{"code", INTEGER}};
    int code = 0;
    int args_count = GET_FIELDS_WITH_CHECK(L, fields, 1, 0);
    if (args_count == 1) {
        code = fields[0].arg.integer;
    }
    ckb_exit(code);
    return 0;
}

int lua_ckb_debug(lua_State *L) {
    FIELD fields[] = {{"msg", STRING}};
    GET_FIELDS_WITH_CHECK(L, fields, 1, 1);
    ckb_debug(fields[0].arg.str);
    return 0;
}

int lua_ckb_load_tx_hash(lua_State *L) {
    return CKB_LOAD_V2(L, ckb_load_tx_hash);
}

int lua_ckb_load_script_hash(lua_State *L) {
    return CKB_LOAD_V2(L, ckb_load_script_hash);
}

int lua_ckb_load_script(lua_State *L) {
    return CKB_LOAD_V2(L, ckb_load_script);
}

int lua_ckb_load_transaction(lua_State *L) {
    return CKB_LOAD_V2(L, ckb_load_transaction);
}

int lua_ckb_load_cell(lua_State *L) { return CKB_LOAD_V4(L, ckb_load_cell); }

int lua_ckb_load_input(lua_State *L) { return CKB_LOAD_V4(L, ckb_load_input); }

int lua_ckb_load_header(lua_State *L) {
    return CKB_LOAD_V4(L, ckb_load_header);
}

int lua_ckb_load_witness(lua_State *L) {
    return CKB_LOAD_V4(L, ckb_load_witness);
}

int lua_ckb_load_cell_data(lua_State *L) {
    return CKB_LOAD_V4(L, ckb_load_cell_data);
}

int lua_ckb_load_cell_by_field(lua_State *L) {
    return CKB_LOAD_V5(L, ckb_load_cell_by_field);
}

int lua_ckb_load_input_by_field(lua_State *L) {
    return CKB_LOAD_V5(L, ckb_load_input_by_field);
}

int lua_ckb_load_header_by_field(lua_State *L) {
    return CKB_LOAD_V5(L, ckb_load_header_by_field);
}

static const luaL_Reg ckb_syscall[] = {
    {"dump", lua_ckb_dump},
    {"exit", lua_ckb_exit},
    {"debug", lua_ckb_debug},
    {"load_tx_hash", lua_ckb_load_tx_hash},
    {"load_script_hash", lua_ckb_load_script_hash},
    {"load_script", lua_ckb_load_script},
    {"load_transaction", lua_ckb_load_transaction},

    {"load_cell", lua_ckb_load_cell},
    {"load_input", lua_ckb_load_input},
    {"load_header", lua_ckb_load_header},
    {"load_witness", lua_ckb_load_witness},
    {"load_cell_data", lua_ckb_load_cell_data},
    {"load_cell_by_field", lua_ckb_load_cell_by_field},
    {"load_input_by_field", lua_ckb_load_input_by_field},
    {"load_header_by_field", lua_ckb_load_header_by_field},
    {NULL, NULL}};

LUAMOD_API int luaopen_ckb(lua_State *L) {
    // create ckb table
    luaL_newlib(L, ckb_syscall);

    SET_FIELD(L, CKB_SUCCESS, "SUCCESS")
    SET_FIELD(L, -CKB_INDEX_OUT_OF_BOUND, "INDEX_OUT_OF_BOUND")
    SET_FIELD(L, -CKB_ITEM_MISSING, "ITEM_MISSING")
    SET_FIELD(L, -CKB_LENGTH_NOT_ENOUGH, "LENGTH_NOT_ENOUGH")
    SET_FIELD(L, -CKB_INVALID_DATA, "INVALID_DATA")
    SET_FIELD(L, -CKB_LUA_OUT_OF_MEMORY, "OUT_OF_MEMORY")

    SET_FIELD(L, CKB_SOURCE_INPUT, "SOURCE_INPUT")
    SET_FIELD(L, CKB_SOURCE_OUTPUT, "SOURCE_OUTPUT")
    SET_FIELD(L, CKB_SOURCE_CELL_DEP, "SOURCE_CELL_DEP")
    SET_FIELD(L, CKB_SOURCE_HEADER_DEP, "SOURCE_HEADER_DEP")
    SET_FIELD(L, CKB_SOURCE_GROUP_INPUT, "SOURCE_GROUP_INPUT")
    SET_FIELD(L, CKB_SOURCE_GROUP_OUTPUT, "SOURCE_GROUP_OUTPUT")

    SET_FIELD(L, CKB_CELL_FIELD_CAPACITY, "CELL_FIELD_CAPACITY")
    SET_FIELD(L, CKB_CELL_FIELD_DATA_HASH, "CELL_FIELD_DATA_HASH")
    SET_FIELD(L, CKB_CELL_FIELD_LOCK, "CELL_FIELD_LOCK")
    SET_FIELD(L, CKB_CELL_FIELD_LOCK_HASH, "CELL_FIELD_LOCK_HASH")
    SET_FIELD(L, CKB_CELL_FIELD_TYPE, "CELL_FIELD_TYPE")
    SET_FIELD(L, CKB_CELL_FIELD_TYPE_HASH, "CELL_FIELD_TYPE_HASH")
    SET_FIELD(L, CKB_CELL_FIELD_OCCUPIED_CAPACITY,
              "CELL_FIELD_OCCUPIED_CAPACITY")

    SET_FIELD(L, CKB_INPUT_FIELD_OUT_POINT, "INPUT_FIELD_OUT_POINT")
    SET_FIELD(L, CKB_INPUT_FIELD_SINCE, "INPUT_FIELD_SINCE")

    SET_FIELD(L, CKB_HEADER_FIELD_EPOCH_NUMBER, "HEADER_FIELD_EPOCH_NUMBER")
    SET_FIELD(L, CKB_HEADER_FIELD_EPOCH_START_BLOCK_NUMBER,
              "HEADER_FIELD_EPOCH_START_BLOCK_NUMBER")
    SET_FIELD(L, CKB_HEADER_FIELD_EPOCH_LENGTH, "HEADER_FIELD_EPOCH_LENGTH")

    // move ckb table to global
    lua_setglobal(L, "ckb");
    return 1;
}
