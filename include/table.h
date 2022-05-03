#ifndef _TABLE_H_
#define _TABLE_H_

#include <stdint.h>
#include "types.h"

// 创建表，可变参数为本表的一行的 types，详见 types.h
// 如果 types 为外键，需要紧跟一个 uint64_t ptr
// 指示外键链接到的表位置
// len(buf) >= 4096+8+2=4106
// 返回：
//    NULL  失败，详见 errno
//    table 指向表头的指针
void* create_table(int fd, char* buf, const char* name, int row_len, const void* list);

// 加载 ptr 位置的表
// len(buf) >= 4096+8+2=4106
// 返回：
//    0   失败，详见 errno
//    ptr 指向表头的指针
void* load_table(int fd, char* buf, uint64_t ptr);

// 获得表名长度，包含结尾0
uint16_t get_table_name_length(void* table);

// 获得表名，写入buf，len(buf) >= len(name)
// 返回：buf
char* get_table_name(void* table, char* buf);

// 获得第 pos 项的索引 ptr
uint64_t get_index_ptr(void* table, uint16_t pos);

// 为 pos 位置的列创建索引。不可用于 0 列，即 pk 列，因为 pk 必有索引
// 返回：
//    NULL 失败，详见 errno
//    ptr  指向索引头的指针
uint64_t add_table_index(int fd, void* table, uint16_t pos);

// 删除 pos 位置的列的索引。不可用于 0 列，即 pk 列，因为 pk 必有索引
// 返回：
//    非 0  失败，详见 errno
//    0     成功
int remove_table_index(int fd, void* table, uint16_t pos);

// 插入一行，如果 pk 有值则替换
// 如果当前项有 nullable 属性，需要在此项之前
// 加一个 int isavailable，标记本项是否有值
// 如果 isavailable==0，后面不再跟有本项数据
// 如果 isavailable!=0，则在后面附加数据
// 如果 val 不为 string，直接装填其值
// 否则，值是指向 string 的指针 (const char*)
// 如果是 binary，需要在指针之前提供一个 uint32 参数
// 说明 binary 的大小
// 返回：
//    0   失败，详见 errno
//    ptr 本行插入的位置
uint64_t insert_row(int fd, void* table, int row_len, const void* list);

// 根据主键的匹配值查找行
// 如果主键不为 string，k 直接装填其值
// 否则，k 是指向 string 的指针 (const char*)
// 返回：
//    0  失败，详见 errno
//    ptr  行所在位置
uint64_t find_row_by_pk(int fd, void* table, key_t k);

// 根据任意匹配值遍历查找行
// 可变参数两两成对，uint16_t pos + key_t val
// 如果 val 不为 string，直接装填其值
// 否则，值是指向 string 的指针 (const char*)
// f 为遍历函数，入参为本行 ptr，返回非 0 值中断遍历
// 返回：
//    非 0  失败，详见 errno
//    0     成功
int find_row_by(int fd, void* table, int (*f)(uint64_t), int row_len, const void* list);

// 根据主键的匹配值删除行
// 如果主键不为 string，k 直接装填其值
// 否则，k 是指向 string 的指针 (const char*)
// 返回：
//    非 0  失败，详见 errno
//    0     成功
int remove_row_by_pk(int fd, void* table, key_t k);

// 根据任意匹配值删除行
// 可变参数两两成对，uint16_t pos + key_t val
// 如果 val 不为 string，直接装填其值
// 否则，值是指向 string 的指针 (const char*)
// 返回：
//    非 0  失败，详见 errno
//    0     成功
int remove_row_by(int fd, void* table, int row_len, const void* list);

#endif