#ifndef _TYPE_INT8_H_
#define _TYPE_INT8_H_

#include <stdint.h>
#include "../types.h"

#define INT8_INDEX_SZ (256*8)

// len(buf) >= INT8_INDEX_SZ+10
void* create_int8_index(int fd, void* buf);

// len(buf) >= INT8_INDEX_SZ+10
void* load_int8_index(int fd, uint64_t ptr, void* buf);

int remove_int8_index(int fd, void* index);

uint64_t count_int8_items(int fd, void* index);

int insert_int8_item(int fd, void* index, key_t k, uint64_t ptr);

uint64_t find_item_by_int8_key(int fd, void* index, key_t k);

uint64_t remove_item_by_int8_key(int fd, void* index, key_t k);

#endif