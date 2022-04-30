#ifndef _PAGE_H_
#define _PAGE_H_

#include <stdio.h>

#ifndef PAGESZ
    #define PAGESZ 4096
#endif

// 获取文件中的第一个空闲页，并将其 mmap 到内存
// 如果现有空闲均无整页，则新分配一页
// 返回：
//    EOF 错误
void* alloc_page(int fd);

// 释放分配的页，将其标记为空闲以备使用
// 返回：
//    EOF 错误
int free_page(int fd, void* page);

// 获取文件中的第一个满足 size 大小的块
// 返回：
//    EOF 错误
void* alloc_block(int fd, size_t size);

// 释放块，并将其标记为空闲以备使用
// 值得注意的是，只有在本页最后一块
// 也被释放后，这一页才会被真正释放
// 返回：
//    EOF 错误
int free_block(int fd, void* page);

#endif