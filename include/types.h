#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>
#include <errno.h>

#define TYPE_INT8   ((uint8_t)0)
#define TYPE_INT16  ((uint8_t)1)
#define TYPE_INT32  ((uint8_t)2)
#define TYPE_INT64  ((uint8_t)3)
#define TYPE_FLOAT  ((uint8_t)4)
#define TYPE_DOUBLE ((uint8_t)5)
#define TYPE_STRING ((uint8_t)6)
#define TYPE_BINARY ((uint8_t)7)

#define EXTYPE_NULL         ((uint8_t)0x00)
#define EXTYPE_UNIQUE       ((uint8_t)0x40)
#define EXTYPE_NONNULL      ((uint8_t)0x80)
#define EXTYPE_FOREIGNKEY   ((uint8_t)0xc0)

typedef uint8_t type_t;
typedef uint64_t key_t;

// 获得本类型 index 相对于 buffer 头的偏移
int type_offset(type_t t);

// 为类型 type 创建索引
// 返回：索引头节点的指针 index
void* create_index(int fd, type_t t, void* buf);

// 加载类型 type 的索引
// 返回：索引头节点的指针 index
void* load_index(int fd, type_t t, uint64_t ptr, void* buf);

// 移除 index 并释放空间
int remove_index(int fd, type_t t, void* index);

// 统计索引条数
// 返回：索引条数
uint64_t count_items(int fd, type_t t, void* index);

// 插入一条索引
int insert_item(int fd, type_t t, void* index, key_t k, uint64_t ptr);

// 使用索引查找目标
// 返回：ptr
uint64_t find_item_by_key(int fd, type_t t, void* index, key_t k);

// 使用索引删除项
// 返回：ptr
uint64_t remove_item_by_key(int fd, type_t t, void* index, key_t k);

#endif