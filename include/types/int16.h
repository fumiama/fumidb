#ifndef _TYPE_INT16_H_
#define _TYPE_INT16_H_

#include <stdint.h>
#include "../types.h"

#define INT16_INDEX_SZ  ( 8*3 + 256*8 )
#define INT16_BITMAP_SZ ( 65536/8 )
#define INT16_CHAIN_SZ ( (256+1)*8 )

// len(buf) >= INT16_INDEX_SZ+10 + INT16_BITMAP_SZ+8*2 = 10290
// &buf[0]    ~ &buf[2081] is index, index = buf+10
// &buf[2082] ~ &buf[6185] is the first page of bitmap, ptr = buf+2090
// &buf[6186] ~ &buf[10289] is the second page of bitmap, ptr = buf+6194
// 返回：index = buf+10
void* create_int16_index(int fd, void* buf);

// len(buf) >= INT16_INDEX_SZ+10 + INT16_BITMAP_SZ+8*2 = 10290
// &buf[0]    ~ &buf[2081] is index, index = buf+10
// &buf[2082] ~ &buf[6185] is the first page of bitmap, ptr = buf+2090
// &buf[6186] ~ &buf[10289] is the second page of bitmap, ptr = buf+6194
// 返回：index = buf+10
void* load_int16_index(int fd, uint64_t ptr, void* buf);

int remove_int16_index(int fd, void* index);

uint64_t count_int16_items(int fd, void* index);

int insert_int16_item(int fd, void* index, key_t k, uint64_t ptr);

uint64_t find_item_by_int16_key(int fd, void* index, key_t k);

uint64_t remove_item_by_int16_key(int fd, void* index, key_t k);

#endif