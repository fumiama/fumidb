#ifndef _TYPE_INT64_H_
#define _TYPE_INT64_H_

#include <stdint.h>
#include "../types.h"

#define INT64_BUCKET_SZ (1024ull)
#define INT64_BUCKET_MAX_ITEM_SZ (255ull)
#define INT64_INDEX_SZ ((INT64_BUCKET_SZ+2)*(PAGESZ+8))
#define INT64_DIGEST_FACTOR (18446744073709551557ull)

// len(buf) >= INT64_INDEX_SZ
// 1024个哈希桶, 多出来2个是机动空间, 用于加载溢出桶
void* create_int64_index(int fd, void* buf);

void* load_int64_index(int fd, uint64_t ptr, void* buf);

int remove_int64_index(int fd, void* index);

uint64_t count_int64_items(int fd, void* index);

int insert_int64_item(int fd, void* index, key_t k, uint64_t ptr);

uint64_t find_item_by_int64_key(int fd, void* index, key_t k);

uint64_t remove_item_by_int64_key(int fd, void* index, key_t k);

#endif