
TARGET := riscv64-unknown-linux-gnu
CC := $(TARGET)-gcc
LD := $(TARGET)-gcc
OBJCOPY := $(TARGET)-objcopy

CFLAGS := -fPIC -Os -fno-builtin -nostdinc -nostdlib -nostartfiles -fvisibility=hidden -fdata-sections -ffunction-sections -I lualib -I include/ckb-c-stdlib -I include/ckb-c-stdlib/libc -I include/ckb-c-stdlib/molecule -Wall -Werror -Wno-nonnull -Wno-nonnull-compare -Wno-unused-function -g

LDFLAGS := -nostdlib -nostartfiles -fno-builtin -Wl,-static -fdata-sections -ffunction-sections -Wl,--gc-sections

# docker pull nervos/ckb-riscv-gnu-toolchain:gnu-bionic-20191012
BUILDER_DOCKER := nervos/ckb-riscv-gnu-toolchain@sha256:aae8a3f79705f67d505d1f1d5ddc694a4fd537ed1c7e9622420a470d59ba2ec3
PORT ?= 9999

all: lualib/liblua.a build/lua-loader

all-via-docker:
	docker run --rm -v `pwd`:/code ${BUILDER_DOCKER} bash -c "cd /code && make"

lualib/liblua.a:
	make -C lualib liblua.a

build/lua-loader.o: lua-loader/lua-loader.c
	$(CC) -c $(CFLAGS) -o $@ $<

# We need the soft floating point number support from libgcc.
# Note when -nostdlib is specified, libgcc is not linked to the program automatically.
# Also note libgcc.a must be appended to the file list, simply -lgcc does not for some reason.
# It seems gcc does not search libgcc in the install path.
build/lua-loader: build/lua-loader.o lualib/liblua.a
	$(LD) $(LDFLAGS) -o $@ $^ $(shell $(CC) --print-search-dirs | sed -n '/install:/p' | sed 's/install:\s*//g')libgcc.a
	cp $@ $@.debug
	$(OBJCOPY) --strip-debug --strip-all $@

fmt:
	clang-format -style="{BasedOnStyle: google, IndentWidth: 4, SortIncludes: false}" -i lualib/*.c lualib/*.h lua-loader/*.c

clean-local:
	rm -f build/*.o
	rm -f build/lua-loader
	rm -f build/lua-loader.debug

clean: clean-local
	make -C lualib clean

run-docker:
	docker run --rm --network host -it -v `pwd`:/code ${BUILDER_DOCKER} bash

run-gdb:
	riscv64-unknown-linux-gnu-gdb -ex "target remote 127.0.0.1:${PORT}" build/lua-loader.debug
