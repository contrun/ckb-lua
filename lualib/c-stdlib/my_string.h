#ifndef LUA_C_STDLIB_STRING_H_
#define LUA_C_STDLIB_STRING_H_

#include <stddef.h>

char *strchr (const char *, int);
int strncmp(const char *_l, const char *_r, size_t n);
char *strpbrk (const char *, const char *);
size_t strcspn (const char *, const char *);
size_t strspn (const char *, const char *);
void *memchr (const void *, int, size_t);

int strcoll (const char *, const char *);

char *strerror (int);

char *strstr(const char *str, const char *substr);

#endif /* LUA_C_STDLIB_STRING_H_ */
