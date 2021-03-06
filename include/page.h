#ifndef _PAGE_H_
#define _PAGE_H_

#include <stdio.h>

#ifndef PAGESZ
    #define PAGESZ 4096
#endif

// 获取文件中的第一个空闲页，并将其读到 buf
// buf 的大小为 PAGESZ+8 = 4104
// 如果现有空闲均无整页，则新分配一页
// 返回指针 page = buf+8
// 其 -8 字节记录了本页在文件的偏移
// 以备释放之用
// 返回：
//    NULL 错误，参见 errno
//    page 指针
void* alloc_page(int fd, void* buf);

// 获取 ptr 处的页
// buf 的大小为 PAGESZ+8 = 4104
// 返回：
//    NULL 错误，参见 errno
//    page 指针
void* get_page(int fd, uint64_t ptr, void* buf);

// 刷新一页
// 返回：
//    EOF lseek 错误，参见 errno
//    0/1 write size != PAGESZ
int sync_page(int fd, void* page);

// 释放分配的页，将其标记为空闲以备使用
// 同时将数据刷新到文件
// 返回：
//    EOF 错误，参见 errno
//    0   成功
int free_page(int fd, void* page);

// 获取文件中的第一个满足 size 大小的块的位置
// 返回的块 blk = buf + 10
// len(buf) >= size + 8 + 2
// 开头的 8+2 字节记录了真正的块位置与大小以备释放
// 返回的 ptr 已经跳过这 10 个字节
// 返回：
//    NULL 错误，参见 errno
//    blk  指针
void* alloc_block(int fd, uint16_t size, void* buf);

// 获取 ptr 处的块
// 返回的块 blk = buf + 10
// len(buf) >= size + 8 + 2
// 返回：
//    NULL 错误，参见 errno
//    blk  指针
void* get_block(int fd, uint16_t size, uint64_t ptr, void* buf);

// 刷新 block 到文件
// 返回：
//    EOF lseek 错误或 size 过大，参见 errno
//    0/1 write size != real size
int sync_block(int fd, void* blk);

// 释放块，并将其标记为空闲以备使用
// 如果该块前后也有空闲且不跨过4096
// 将会被合并
// 返回：
//    EOF 错误，参见 errno
//    0   成功
int free_block(int fd, void* blk);

// 将 ptr 处的块纳入空闲块链表
// 返回：
//    EOF lseek 错误或 size 过大，参见 errno
//    0/1 write size != real size
int add_block(int fd, uint16_t size, uint64_t ptr);

#endif