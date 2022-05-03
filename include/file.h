#ifndef _FILE_H_
#define _FILE_H_

#include <stdint.h>

#define HEADERSZ 256

// 初始化并写入数据库文件头
// 返回：
//    0   成功
//    1   write 失败
//    EOF lseek 失败
int init_file_header_page(int fd);

// 获得数据库版本
uint16_t get_db_version(int fd);

// 设置 ptr of unused blk 字段
// 返回：
//    0   成功
//    1   write 失败
//    EOF lseek 失败
int set_first_unused_block(int fd, uint64_t ptr);

// 获得 ptr of unused blk 字段
uint64_t get_first_unused_block(int fd);

uint64_t get_next_unused_block(int fd, uint64_t ptr);

// 设置 ptr of next table 字段
// 返回：
//    0   成功
//    1   write 失败
//    EOF lseek 失败
int set_first_table(int fd, uint64_t ptr);

// 获得 ptr of next table 字段
uint64_t get_first_table(int fd);

uint64_t get_next_table(int fd, uint64_t ptr);

#endif