#define CKB_C_STDLIB_PRINTF
#include <stdio.h>

#include "blake2b.h"
#include "blockchain.h"
#include "ckb_syscalls.h"
#include "ckb_dlfcn.h"

#define MAX_CODE_SIZE (1024 * 1024)
#define MAX_SCRIPT_SIZE (32 * 1024)

#define RESERVED_ARGS_SIZE 2
#define BLAKE2B_BLOCK_SIZE 32
#define HASH_TYPE_SIZE 1

enum ErrorCode {
    // inherit from simple_udt
    ERROR_ENCODING = -2,
    ERROR_SCRIPT_TOO_LONG = -21,

    // error code is starting from 40, to avoid conflict with
    // common error code in other scripts.
    ERROR_CANT_LOAD_LIB = 40,
    ERROR_LIB_MALFORMED,
    ERROR_CANT_FIND_SYMBOL,
    ERROR_INVALID_ARGS_FORMAT,
};

uint8_t code_buff[MAX_CODE_SIZE] __attribute__((aligned(RISCV_PGSIZE)));

int get_dylib_handle(void** handle) {
    unsigned char script[MAX_SCRIPT_SIZE];
    uint64_t len = MAX_SCRIPT_SIZE;
    int err = ckb_load_script(script, &len, 0);
    if (err != 0) {
        printf("loading script error %d\n", err);
        return err;
    }
    if (len > MAX_SCRIPT_SIZE) {
        return ERROR_SCRIPT_TOO_LONG;
    }

    mol_seg_t script_seg;
    script_seg.ptr = (uint8_t*)script;
    script_seg.size = len;

    if (MolReader_Script_verify(&script_seg, false) != MOL_OK) {
        printf("verifying script failed\n");
        return ERROR_ENCODING;
    }

    // The script arguments are in the following format
    // <reserved args, 2 bytes> <code hash of the share library, 32 bytes>
    // <hash type of shared library, 1 byte>
    mol_seg_t args_seg = MolReader_Script_get_args(&script_seg);
    mol_seg_t args_bytes_seg = MolReader_Bytes_raw_bytes(&args_seg);

    if (args_bytes_seg.size <
        RESERVED_ARGS_SIZE + BLAKE2B_BLOCK_SIZE + HASH_TYPE_SIZE) {
        return ERROR_INVALID_ARGS_FORMAT;
    }

    uint8_t* code_hash = args_bytes_seg.ptr + RESERVED_ARGS_SIZE;
    uint8_t hash_type =
        *(args_bytes_seg.ptr + RESERVED_ARGS_SIZE + BLAKE2B_BLOCK_SIZE);

    size_t code_buff_size = MAX_CODE_SIZE;
    size_t consumed_size = 0;

    err = ckb_dlopen2(code_hash, hash_type, code_buff, code_buff_size, handle,
                      &consumed_size);
    if (err != 0) {
        printf("dl_opening error: %d\n", err);
        return err;
    }
    if (handle == NULL) {
        printf("dl_opening error, can not load library\n");
        return ERROR_CANT_LOAD_LIB;
    }
    if (consumed_size % RISCV_PGSIZE != 0) {
        printf("dl_opening error, library malformed\n");
        return ERROR_LIB_MALFORMED;
    }
    return 0;
}

void must_get_dylib_handle(void** handle) {
    int err = get_dylib_handle(handle);
    if (err != 0) {
        ckb_exit(err);
    }
}

void* must_load_function(void* handle, char* name) {
    void* func = ckb_dlsym(handle, name);
    if (func == NULL) {
        printf("dl_opening error, can't find symbol %s\n", name);
        ckb_exit(ERROR_CANT_FIND_SYMBOL);
    }
    return func;
}

typedef void* (*CreateLuaInstanceFuncType)(uintptr_t min, uintptr_t max);
typedef int (*EvaluateLuaCodeFuncType)(void* l, const char* code,
                                       size_t code_size, char* name);
typedef void (*CloseLuaInstanceFuncType)(void* l);
typedef void (*ToggleExitFuncType)(void* l, int enabled);

void run_lua_test_code(void* handle, int n) {
    CreateLuaInstanceFuncType create_func =
        must_load_function(handle, "lua_create_instance");
    EvaluateLuaCodeFuncType evaluate_func =
        must_load_function(handle, "lua_run_code");
    CloseLuaInstanceFuncType close_func =
        must_load_function(handle, "lua_close_instance");

    printf("Running test %s\n", __func__);

    const size_t mem_size = 1024 * 512;
    uint8_t mem[mem_size];

    size_t i = 0;
    // At least run lua code once.
    do {
        void* l = create_func((uintptr_t)mem, (uintptr_t)(mem + mem_size));
        if (l == NULL) {
            printf("creating lua instance failed\n");
            return;
        }
        const char* code = "print('hello world')";
        size_t code_size = strlen(code);
        const size_t times = 10;
        printf("evaluating lua code for %zu times\n", times);
        size_t j = 0;
        do {
            int ret = evaluate_func(l, code, code_size, "test");
            if (ret != 0) {
                printf("evaluating lua code failed: %d\n", ret);
                return;
            }
            j++;
        } while (j < times);
        close_func(l);
        i++;
    } while (i < n);
}

void test_invalid_lua_code(void* handle) {
    CreateLuaInstanceFuncType create_func =
        must_load_function(handle, "lua_create_instance");
    EvaluateLuaCodeFuncType evaluate_func =
        must_load_function(handle, "lua_run_code");
    CloseLuaInstanceFuncType close_func =
        must_load_function(handle, "lua_close_instance");

    printf("Running test %s\n", __func__);

    const size_t mem_size = 1024 * 512;
    uint8_t mem[mem_size];

    void* l = create_func((uintptr_t)mem, (uintptr_t)(mem + mem_size));
    if (l == NULL) {
        printf("creating lua instance failed\n");
        ckb_exit(-1);
    }
    const char* code = "invalid lua code here";
    size_t code_size = strlen(code);
    int ret = evaluate_func(l, code, code_size, "invalid lua code test");
    if (ret >= 0) {
        close_func(l);
        printf(
            "evaluating lua code failed: returned %d, expecting negative "
            "number\n",
            ret);
        ckb_exit(-1);
    }
    close_func(l);
}

void test_exit_script(void* handle) {
    CreateLuaInstanceFuncType create_func =
        must_load_function(handle, "lua_create_instance");
    EvaluateLuaCodeFuncType evaluate_func =
        must_load_function(handle, "lua_run_code");
    CloseLuaInstanceFuncType close_func =
        must_load_function(handle, "lua_close_instance");

    printf("Running test %s\n", __func__);

    const size_t mem_size = 1024 * 512;
    uint8_t mem[mem_size];

    void* l = create_func((uintptr_t)mem, (uintptr_t)(mem + mem_size));
    if (l == NULL) {
        printf("creating lua instance failed\n");
        ckb_exit(-1);
    }
    const char* code =
        "local function exit_script()\n"
        "  print('Prepared to exit script')\n"
        "  ckb.exit_script(42)\n"
        "  print('Code after exit_script should be unreachable')\n"
        "end\n"
        "exit_script()";
    size_t code_size = strlen(code);
    int ret = evaluate_func(l, code, code_size, "exit_script test");
    if (ret != 42) {
        close_func(l);
        printf("evaluating lua code failed: returned %d, expecting 42\n", ret);
        ckb_exit(-1);
    }
    close_func(l);
}

void test_exit(void* handle) {
    CreateLuaInstanceFuncType create_func =
        must_load_function(handle, "lua_create_instance");
    EvaluateLuaCodeFuncType evaluate_func =
        must_load_function(handle, "lua_run_code");
    CloseLuaInstanceFuncType close_func =
        must_load_function(handle, "lua_close_instance");
    ToggleExitFuncType toggle_exit_func =
        must_load_function(handle, "lua_toggle_exit");

    printf("Running test %s\n", __func__);

    const size_t mem_size = 1024 * 512;
    uint8_t mem[mem_size];

    void* l = create_func((uintptr_t)mem, (uintptr_t)(mem + mem_size));
    if (l == NULL) {
        printf("creating lua instance failed\n");
        ckb_exit(-1);
    }
    const char* code = "ckb.exit(0)";
    size_t code_size = strlen(code);
    int ret = evaluate_func(l, code, code_size, "exit not enabled");
    if (ret >= 0) {
        close_func(l);
        printf(
            "evaluating lua code returned %d, expecting negative "
            "number as exit is not enabled\n",
            ret);
        ckb_exit(-1);
    }
    printf("before toggle exit func: returned %d", ret);
    toggle_exit_func(l, 1);
    printf("after toggle exit func");
    ret = evaluate_func(l, code, code_size, "exit enabled");

    // Below should never run, as the whole vm exited.
    printf("Code after exit_script should be unreachable\n");
    printf("Unexpected return from evaluating lua code, return code %d", ret);
    close_func(l);
}

int main(int argc, char* argv[]) {
    void* handle;
    must_get_dylib_handle(&handle);

    int n = 0;
    size_t optind;
    for (optind = 0; optind < argc && argv[optind][0] == '-'; optind++) {
        switch (argv[optind][1]) {
            case 'c': {
                n = _atoi((const char**)&argv[optind + 1]);
                optind = optind + 1;
                break;
            }
            default:
                printf("Usage: [-c number_of_times_to_run]\n");
                ckb_exit(-1);
        }
    }
    argv += optind;

    run_lua_test_code(handle, n);

    test_invalid_lua_code(handle);

    test_exit_script(handle);

    // Must be the last test to run, as it will stop the execution.
    test_exit(handle);
}
