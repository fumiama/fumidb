#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>
#include <errno.h>

#define TYPE_INT8   0
#define TYPE_INT16  1
#define TYPE_INT32  2
#define TYPE_INT64  3
#define TYPE_FLOAT  4
#define TYPE_DOUBLE 5
#define TYPE_STRING 6
#define TYPE_BINARY 7

#define EXTYPE_NULL         0x00
#define EXTYPE_UNIQUE       0x40
#define EXTYPE_NONNULL      0x80
#define EXTYPE_FOREIGNKEY   0xc0

typedef uint8_t type_t;
typedef uint64_t key_t;

// 为类型 type 创建索引
// 返回：索引头节点的指针 index
void* create_index(int fd, type_t t);

// 加载类型 type 的索引
// 返回：索引头节点的指针 index
void* load_index(int fd, type_t t, uint64_t ptr);

// 插入一条索引
int insert_item(int fd, type_t t, void* index, key_t k, uint64_t ptr);

// 使用索引查找目标
// 返回：ptr
uint64_t find_item_by_key(int fd, type_t t, void* index, key_t k);

// 使用索引删除项
int remove_item_by_key(int fd, type_t t, void* index, key_t k);

#endif