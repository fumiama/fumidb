#ifndef _TYPE_INT32_H_
#define _TYPE_INT32_H_

#include <stdint.h>
#include "../types.h"

#define INT32_BUCKET_SZ (256ull)
#define INT32_BUCKET_MAX_ITEM_SZ (340ull)
#define INT32_INDEX_SZ ((INT32_BUCKET_SZ+2)*(PAGESZ+8))
#define INT32_DIGEST_FACTOR (11454191981ull)

// len(buf) >= INT32_INDEX_SZ = (PAGESZ+8)*(256+2) = 4104*258
// 256个哈希桶, 多出来2个是机动空间, 用于加载溢出桶
void* create_int32_index(int fd, void* buf);

void* load_int32_index(int fd, uint64_t ptr, void* buf);

int remove_int32_index(int fd, void* index);

uint64_t count_int32_items(int fd, void* index);

int insert_int32_item(int fd, void* index, key_t k, uint64_t ptr);

uint64_t find_item_by_int32_key(int fd, void* index, key_t k);

uint64_t remove_item_by_int32_key(int fd, void* index, key_t k);

#endif