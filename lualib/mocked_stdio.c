
#include "mocked_stdio.h"

FILE *stdin;
FILE *stdout;
FILE *stderr;

int remove(const char *__filename) { return 0; }

int rename(const char *__old, const char *__new) { return 0; }

FILE *tmpfile(void) { return 0; }

char *tmpnam(char *__s) { return 0; }

char *tempnam(const char *__dir, const char *__pfx) { return 0; }

int fclose(FILE *__stream) { return 0; }

int fflush(FILE *__stream) { return 0; }

FILE *fopen(const char *__filename, const char *__modes) { return 0; }

FILE *freopen(const char *__filename, const char *__modes, FILE *__stream) {
    return 0;
}

void setbuf(FILE *__stream, char *__buf) {}

int setvbuf(FILE *__stream, char *__buf, int __modes, size_t __n) { return 0; }

int fprintf(FILE *__stream, const char *__format, ...) { return 0; }

#ifndef CKB_C_STDLIB_PRINTF
int printf(const char *__format, ...) { return 0; }
#endif

int sprintf(char *__s, const char *__format, ...) { return 0; }

int vfprintf(FILE *__s, const char *__format, ...) { return 0; }
int vprintf(const char *__format, ...) { return 0; }
int vsprintf(char *__s, const char *__format, ...) { return 0; }

int snprintf(char *__s, size_t __maxlen, const char *__format, ...) {
    return 0;
}

int vsnprintf(char *__s, size_t __maxlen, const char *__format, ...) {
    return 0;
}
int fscanf(FILE *__stream, const char *__format, ...) { return 0; }

int scanf(const char *__format, ...) { return 0; }

int sscanf(const char *__s, const char *__format, ...) { return 0; };

int fgetc(FILE *__stream) { return 0; }

int getc(FILE *__stream) { return 0; }

int getchar(void) { return 0; }

int fputc(int __c, FILE *__stream) { return 0; }

int putc(int __c, FILE *__stream) { return 0; }

int putchar(int __c) { return 0; }

char *fgets(char *__s, int __n, FILE *__stream) { return 0; }

char *gets(char *__s) { return 0; }

int getline(char **__lineptr, size_t *__n, FILE *__stream) { return 0; }

int fputs(const char *__s, FILE *__stream) { return 0; }

int puts(const char *__s) { return 0; }

int ungetc(int __c, FILE *__stream) { return 0; }

size_t fread(void *__ptr, size_t __size, size_t __n, FILE *__stream) {
    return 0;
}

size_t fwrite(const void *__ptr, size_t __size, size_t __n, FILE *__s) {
    return 0;
}

int fseek(FILE *__stream, long int __off, int __whence) { return 0; }

long int ftell(FILE *__stream) { return 0; }

void rewind(FILE *__stream) {}

void clearerr(FILE *__stream) {}

int feof(FILE *__stream) { return 0; }

int ferror(FILE *__stream) { return 0; }

void perror(const char *__s) {}

int fileno(FILE *__stream) { return 0; }

FILE *popen(const char *__command, const char *__modes) { return 0; }

int pclose(FILE *__stream) { return 0; }
