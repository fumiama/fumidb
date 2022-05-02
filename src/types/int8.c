#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "../../include/page.h"
#include "../../include/types/int8.h"

void* create_int8_index(int fd, void* buf) {
    uint64_t* blk = alloc_block(fd, INT8_INDEX_SZ, buf);
    if(blk == NULL) return 0;
    memset(blk, 0, INT8_INDEX_SZ);
    sync_block(fd, blk);
    return blk;
}

void* load_int8_index(int fd, uint64_t ptr, void* buf) {
    return get_block(fd, INT8_INDEX_SZ, ptr, buf);
}

int remove_int8_index(int fd, void* index) {
    return free_block(fd, index);
}

uint64_t count_int8_items(int fd, void* index) {
    uint64_t cnt = 0;
    for(int i = 0; i < 256; i++) cnt += !!(((uint64_t*)index)[i]);
    return cnt;
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

uint64_t remove_item_by_int8_key(int fd, void* index, key_t k) {
    uint8_t key = (uint8_t)k;
    uint64_t ptr = ((uint64_t*)index)[key];
    ((uint64_t*)index)[key] = 0;
    return ptr;
}
