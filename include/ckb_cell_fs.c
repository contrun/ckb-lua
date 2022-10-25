#include <stdlib.h>
#include <string.h>

#include "ckb_cell_fs.h"

// TODO: use a linked list of CellFileSystem so that we can use files from
// multiple cells
static CellFileSystem CELL_FILE_SYSTEM;

int get_file(const CellFileSystem *fs, const char *filename, FSFile **f) {
    FSFile *file = malloc(sizeof(FSFile));
    if (file == 0) {
        return -1;
    }
    for (uint32_t i = 0; i < fs->count; i++) {
        FSEntry entry = fs->files[i];
        if (strcmp(filename, fs->start + entry.filename.offset) == 0) {
            // TODO: check the memory addresses are legal
            file->filename = filename;
            file->size = entry.content.length;
            file->content = fs->start + entry.content.offset;
            file->rc = 1;
            *f = file;
            return 0;
        }
    }
    return -1;
}

int ckb_get_file(const char *filename, FSFile **file) {
    return get_file(&CELL_FILE_SYSTEM, filename, file);
}

int load_fs(CellFileSystem *fs, void *buf, uint64_t buflen) {
    if (buf == NULL || buflen < sizeof(fs->count)) {
        return -1;
    }

    fs->count = *(uint32_t *)buf;
    if (fs->count == 0) {
        fs->files = NULL;
        fs->start = NULL;
        return 0;
    }

    fs->files = (FSEntry *)malloc(sizeof(FSEntry) * fs->count);
    if (fs->files == NULL) {
        return -1;
    }
    fs->start = buf + sizeof(fs->count) + (sizeof(FSEntry) * fs->count);

    FSEntry *entries = (FSEntry *)((char *)buf + sizeof(fs->count));
    for (uint32_t i = 0; i < fs->count; i++) {
        FSEntry entry = entries[i];
        fs->files[i] = entry;
    }

    return 0;
}

int ckb_load_fs(void *buf, uint64_t buflen) {
    int ret = load_fs(&CELL_FILE_SYSTEM, buf, buflen);
    return ret;
}
