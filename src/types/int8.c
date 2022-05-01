#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "../../include/page.h"
#include "../../include/types/int8.h"

void* create_int8_index(int fd) {
    uint64_t* blk = alloc_block(fd, INT8_INDEX_SZ);
    if(blk == NULL) return 0;
    memset(blk, 0, INT8_INDEX_SZ);
    sync_block(fd, blk);
    return blk;
}

void* load_int8_index(int fd, uint64_t ptr) {
    return get_block(fd, INT8_INDEX_SZ, ptr);
}

int insert_int8_item(int fd, void* index, key_t k, uint64_t ptr) {
    uint8_t key = (uint8_t)k;
    ((uint64_t*)index)[key] = ptr;
    return sync_block(fd, index);
}

uint64_t find_item_by_int8_key(int fd, void* index, key_t k) {
    uint8_t key = (uint8_t)k;
    return ((uint64_t*)index)[key];
}

int remove_item_by_int8_key(int fd, void* index, key_t k) {
    uint8_t key = (uint8_t)k;
    ((uint64_t*)index)[key] = 0;
    return 0;
}
