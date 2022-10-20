#define CKB_C_STDLIB_PRINTF
#define CKB_C_STDLIB_MALLOC
#define PRINTF_DISABLE_SUPPORT_FLOAT

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ckb_syscalls.h"
#include "helper.h"

#define COUNT_OF(a) (sizeof(a) / sizeof(a[0]))

void check_heap(void) {
  int i;
  for (i = 0; i < 64; i++) {
    // printf("bit is %d", mal.binmap & 1ULL << i);
    // should be all zero
    if (mal.bins[i].head != CKB_BIN_TO_CHUNK(i) && mal.bins[i].head) {
      if (!(mal.binmap & 1ULL << i)) {
        printf("missing from binmap!\n");
        ckb_exit(-2);
      }
    } else if (mal.binmap & 1ULL << i) {
      printf("binmap wrongly contains %d!\n", i);
      ckb_exit(-2);
    }
  }
}

void reset(void) {
  check_heap();
  s_program_break = 0;
  memset(&mal, 0, sizeof(mal));
  uint64_t p = (uint64_t)_sbrk(0);
  printf("the brk is reset to %lu K", p / 1024);
}

void loop_free(void *array[], size_t array_length, size_t step) {
  for (size_t i = 0; i < array_length; i += step) {
    free(array[i]);
    array[i] = 0;
  }
}

void test_malloc1() {
  printf("test_malloc1 ...");
  void *array[1024] = {0};
  for (size_t i = 0; i < COUNT_OF(array); i++) {
    array[i] = malloc(i * 4);
    ASSERT(array[i] != 0);
  }
  loop_free(array, COUNT_OF(array), 2);
  loop_free(array, COUNT_OF(array), 3);
  loop_free(array, COUNT_OF(array), 4);
  loop_free(array, COUNT_OF(array), 5);
  // free all
  loop_free(array, COUNT_OF(array), 1);

  for (size_t i = 0; i < COUNT_OF(array); i++) {
    array[i] = malloc(i * 4);
    ASSERT(array[i] != 0);
  }
  // free all
  loop_free(array, COUNT_OF(array), 1);
}

void test_malloc_extreme() {
  uint64_t p = (uint64_t)_sbrk(0);
  uint64_t max_brk = 3 * 1024 * 1024;
  void *ptr = malloc(max_brk - p);
  ASSERT_EQ(ptr, 0);
  // the maximum size to allocated can't be determined.
  ptr = malloc(max_brk - p - 1024);
  ASSERT(ptr == 0);
  free(ptr);
}

void test_sbrk() {
  uint64_t base = (uint64_t)_sbrk(0);
  printf("_end = %lu K", base / 1024);
  uint64_t ptr1 = (uint64_t)_sbrk(100);
  uint64_t ptr2 = (uint64_t)_sbrk(200);
  ASSERT_EQ(ptr2 - ptr1, 100);
  uint64_t *ptr3 = _sbrk(8);
  *ptr3 = 100;
  *(ptr3 + 1) = 200;
  uint64_t three_million = 3 * 1024 * 1024;
  uint64_t overflow_ptr = (uint64_t)_sbrk(three_million);
  ASSERT_EQ(overflow_ptr, -1);
}

typedef struct Memory {
  void *ptr;
  size_t size;
} Memory;

static Memory s_allocated[512] = {0};

void test_malloc2() {
  printf("test_malloc2 ...");
  static size_t COUNT = COUNT_OF(s_allocated);
  for (size_t i = 0; i < 100000000; i++) {
    void *p = malloc(i);
    if (!p) {
      break;
    }
    size_t index = i % COUNT;
    if (s_allocated[index].ptr != 0) {
      free(s_allocated[index].ptr);
    }
    s_allocated[index].ptr = p;
    s_allocated[index].size = i;
    if (i > 0) {
      uint8_t *ptr = (uint8_t *)p;
      ptr[0] = 0xBE;
      ptr[i - 1] = 0xBE;
    }
  }
  for (size_t i = 0; i < COUNT; i++) {
    if (s_allocated[i].ptr) {
      free(s_allocated[i].ptr);
      s_allocated[i].ptr = 0;
      s_allocated[i].size = 0;
    }
    if (i % 7 == 0) {
      void *p = malloc(i);
      if (p) {
        s_allocated[i].ptr = p;
        s_allocated[i].size = i;
        if (i > 0) {
          uint8_t *ptr = (uint8_t *)p;
          ptr[0] = 0xBC;
          ptr[i - 1] = 0xBC;
        }
      }
    }
  }
  // free all
  for (size_t i = 0; i < COUNT; i++) {
    if (s_allocated[i].ptr) {
      free(s_allocated[i].ptr);
      s_allocated[i].ptr = 0;
      s_allocated[i].size = 0;
    }
  }

  uint64_t p = (uint64_t)_sbrk(0);
  printf("finally, the brk is at %lu K", p / 1024);
}

void test_realloc() {
  printf("test_realloc ...");
  static size_t COUNT = COUNT_OF(s_allocated);
  for (size_t i = 0; i < 100000000; i++) {
    void *p = malloc(i);
    if (!p) {
      break;
    }
    size_t index = i % COUNT;
    if (s_allocated[index].ptr != 0) {
      free(s_allocated[index].ptr);
      s_allocated[index].ptr = 0;
      s_allocated[index].size = 0;
    }
    p = realloc(p, i * 2);
    if (!p) {
      break;
    }
    s_allocated[index].ptr = p;
    s_allocated[index].size = i * 2;
    if (i > 0) {
      uint8_t *ptr = (uint8_t *)p;
      ptr[0] = 0xB0;
      ptr[2 * i - 1] = 0xB0;
    }
  }
  // free all
  for (size_t i = 0; i < COUNT; i++) {
    if (s_allocated[i].ptr) {
      free(s_allocated[i].ptr);
      s_allocated[i].ptr = 0;
      s_allocated[i].size = 0;
    }
  }
  uint64_t p = (uint64_t)_sbrk(0);
  printf("finally, the brk is at %lu K", p / 1024);
}

int main() {
  test_malloc2();
  reset();
  test_malloc1();
  reset();
  test_malloc1();
  reset();
  test_realloc();
  reset();
  test_sbrk();
  reset();
  test_malloc_extreme();
  reset();

  uint8_t mem[1024*10];
  malloc_config((uintptr_t)mem, (uintptr_t)(mem+1024*10));
  test_malloc2();
  reset();

  return 0;
}
