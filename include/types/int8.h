#ifndef _TYPE_INT8_H_
#define _TYPE_INT8_H_

#include "../types.h"

#define INT8_INDEX_SZ (256*8)

void* create_int8_index(int fd);

void* load_int8_index(int fd, uint64_t ptr);

int insert_int8_item(int fd, void* index, key_t k, uint64_t ptr);

uint64_t find_item_by_int8_key(int fd, void* index, key_t k);

int remove_item_by_int8_key(int fd, void* index, key_t k);

#endif