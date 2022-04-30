// page.c
// 管理文件中的空闲块

#define _GNU_SOURCE /* See feature_test_macros(7) */

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "../include/binary.h"
#include "../include/page.h"

uint8_t nullpage[PAGESZ];

void* alloc_page(int fd) {
    uint64_t ptr, prev_ptr = 8;
    void* page;
    if(lseek(fd, 8, SEEK_SET) < 0) return EOF;
    // 对于 page，只关心位于第一页 8~15 字节的 ptr of unused blk
    readle64(fd, ptr);
    while(ptr) {
        if(!(ptr%PAGESZ)) { // 找到符合要求的页
            page = mmap(NULL, PAGESZ, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, ptr);
            if(page < 0) return page;
            if(lseek(fd, prev_ptr, SEEK_SET) < 0) return EOF;
            write(fd, page, 8); // 从空闲块链表移除本块
            return page;
        }
        prev_ptr = ptr;
        if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
        readle64(fd, ptr);
    }
    ptr = lseek(fd, 0, SEEK_END);
    if(ptr < 0 || !(ptr%PAGESZ)) return EOF;
    if(write(fd, nullpage, PAGESZ) != PAGESZ) return EOF;
    return mmap(NULL, PAGESZ, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, ptr);
}
