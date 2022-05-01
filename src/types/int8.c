#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "../../include/page.h"
#include "../../include/types/int8.h"

uint64_t create_int8_index(int fd) {
    uint64_t* blk = alloc_block(fd, INT8_INDEX_SZ);
    if(blk == NULL) return 0;
    memset(blk, 0, INT8_INDEX_SZ);
    uint64_t index = blk[-1];
    unmount_block(fd, blk);
    return index;
}

int insert_int8_item(int fd, uint64_t index, key_t k, uint64_t ptr) {
    errno = ENOSYS;
    return 0;
}

uint64_t find_item_by_int8_key(int fd, uint64_t index, key_t k) {
    errno = ENOSYS;
    return 0;
}

int remove_item_by_int8_key(int fd, uint64_t index, key_t k) {
    errno = ENOSYS;
    return 0;
}
