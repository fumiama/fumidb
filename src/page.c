// page.c
// 管理文件中的空闲块

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "../include/binary.h"
#include "../include/page.h"

static const uint8_t nullpage[PAGESZ];

void* alloc_page(int fd, void* page) {
    uint64_t ptr = 8, prev_ptr = 0, prev_prev_ptr = 0;
    uint8_t buf[8];
    // 对于 page，只关心位于第一页 8~15 字节的 ptr of unused blk
    while(ptr) {
        if(!(ptr%PAGESZ)) { // 找到符合要求的页
            if(lseek(fd, ptr, SEEK_SET) < 0) return NULL;
            if(read(fd, buf, 8) != 8) return NULL;
            putle64(page, ptr);
            page += 8;
            if(lseek(fd, prev_ptr, SEEK_SET) < 0) return NULL;
            if(write(fd, buf, 8) != 8) return NULL; // 从空闲块链表移除本块
            return page;
        }
        if(prev_prev_ptr && ptr < prev_ptr) { // 不符合顺序，进行一次调整
            lseek(fd, prev_prev_ptr, SEEK_SET);
            putle64(buf, ptr);
            if(write(fd, buf, 8) != 8) return NULL;   // 1->next = 3
            if(lseek(fd, ptr, SEEK_SET) < 0) return NULL;
            prev_prev_ptr = ptr;                            // new1 = 3
            readle64(fd, ptr);                              // ptr(new3) = 3->next
            lseek(fd, -8, SEEK_SET);
            putle64(buf, prev_ptr);                         // new2 = 2
            if(write(fd, buf, 8) != 8) return NULL;   // 3->next = 2
            lseek(fd, prev_ptr, SEEK_SET);
            putle64(buf, ptr);
            if(write(fd, buf, 8) != 8) return NULL;   // 2->next = ptr
            continue;
        }
        prev_prev_ptr = prev_ptr;
        prev_ptr = ptr;
        if(lseek(fd, ptr, SEEK_SET) < 0) return NULL;
        readle64(fd, ptr);
    }
    ptr = lseek(fd, 0, SEEK_END);
    if(ptr < 0) return NULL;
    if(ptr%PAGESZ) { // 文件没有页对齐
        errno = ESPIPE;
        return NULL;
    }
    if(write(fd, nullpage, PAGESZ) != PAGESZ) return NULL;
    putle64(page, ptr);
    return page+8;
}

void* get_page(int fd, uint64_t ptr, void* page) {
    if(ptr%PAGESZ) return NULL;
    if(lseek(fd, ptr, SEEK_SET) < 0) return NULL;
    putle64(page, ptr);
    page += 8;
    if(read(fd, page, PAGESZ) != PAGESZ) return NULL;
    return page;
}

int sync_page(int fd, void* page) {
    uint64_t ptr = le64(page-8);
    if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
    return write(fd, page, PAGESZ) != PAGESZ;
}

int free_page(int fd, void* page) {
    uint64_t ptr = 8, prev_ptr = 0, prev_prev_ptr = 0, page_ptr = le64(page-8);
    uint8_t buf[8];
    while(ptr && ptr < page_ptr) {
        if(prev_ptr == ptr) return EOF;
        if(prev_prev_ptr && ptr < prev_ptr) { // 不符合顺序，进行一次调整
            lseek(fd, prev_prev_ptr, SEEK_SET);
            putle64(buf, ptr);
            if(write(fd, buf, 8) != 8) return EOF; // 1->next = 3
            if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
            prev_prev_ptr = ptr;                   // new1 = 3
            readle64(fd, ptr);                     // ptr(new3) = 3->next
            lseek(fd, -8, SEEK_SET);
            putle64(buf, prev_ptr);                // new2 = 2
            if(write(fd, buf, 8) != 8) return EOF; // 3->next = 2
            lseek(fd, prev_ptr, SEEK_SET);
            putle64(buf, ptr);
            if(write(fd, buf, 8) != 8) return EOF; // 2->next = ptr
            continue;
        }
        prev_prev_ptr = prev_ptr;
        prev_ptr = ptr;
        if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
        readle64(fd, ptr);
    }
    putle64(page, ptr);
    putle16(page+8, PAGESZ);
    putle64(buf, page_ptr);
    lseek(fd, prev_ptr, SEEK_SET);
    if(write(fd, buf, 8) != 8) return EOF; // 将本页附加到链表
    sync_page(fd, page);
    return 0;
}

void* alloc_block(int fd, uint16_t size, void* blk) {
    uint64_t ptr = 8, prev_ptr = 0, prev_prev_ptr = 0;
    uint8_t buf[8];
    uint16_t blksz;

    if(size > PAGESZ) return NULL;
    // 对于 page，只关心位于第一页 8~15 字节的 ptr of unused blk
    while(ptr) {
        if(lseek(fd, ptr, SEEK_SET) < 0) return NULL;
        if(read(fd, buf, 8) != 8) return NULL;
        readle16(fd, blksz);
        if(blksz >= size) { // 找到符合要求的块
            if(blksz - size > 10) { // 分裂块
                lseek(fd, ptr+size, SEEK_SET);
                write(fd, buf, 8);
                blksz -= size;
                putle16(buf, blksz);
                write(fd, buf, 2);
                putle64(buf, ptr+size);
            }
            putle64(blk, ptr);
            putle16(blk+8, size);
            blk += 10;
            if(lseek(fd, prev_ptr, SEEK_SET) < 0) return NULL;
            if(write(fd, buf, 8) != 8) return NULL; // 从空闲块链表移除本块
            return blk;
        }
        if(prev_prev_ptr && ptr < prev_ptr) { // 不符合顺序，进行一次调整
            lseek(fd, prev_prev_ptr, SEEK_SET);
            putle64(buf, ptr);
            if(write(fd, buf, 8) != 8) return NULL;   // 1->next = 3
            if(lseek(fd, ptr, SEEK_SET) < 0) return NULL;
            prev_prev_ptr = ptr;                            // new1 = 3
            readle64(fd, ptr);                              // ptr(new3) = 3->next
            lseek(fd, -8, SEEK_SET);
            putle64(buf, prev_ptr);                         // new2 = 2
            if(write(fd, buf, 8) != 8) return NULL;   // 3->next = 2
            lseek(fd, prev_ptr, SEEK_SET);
            putle64(buf, ptr);
            if(write(fd, buf, 8) != 8) return NULL;   // 2->next = ptr
            continue;
        }
        prev_prev_ptr = prev_ptr;
        prev_ptr = ptr;
        if(lseek(fd, ptr, SEEK_SET) < 0) return NULL;
        readle64(fd, ptr);
    }
    ptr = lseek(fd, 0, SEEK_END);
    if(ptr < 0) return NULL;
    if(ptr%PAGESZ) { // 文件没有页对齐
        errno = ESPIPE;
        return NULL;
    }
    if(write(fd, nullpage, PAGESZ) != PAGESZ) return NULL;
    if(PAGESZ-size > 10) { // 回收冗余
        if(lseek(fd, prev_ptr, SEEK_SET) < 0) return NULL;
        putle64(buf, ptr+size);
        if(write(fd, buf, 8) != 8) return NULL;
        if(lseek(fd, ptr+size+8, SEEK_SET) < 0) return NULL;
        putle16(buf, PAGESZ - size);
        if(write(fd, buf, 2) != 2) return NULL;
    }
    putle64(blk, ptr);
    putle16(blk+8, size);
    return blk+10;
}

void* get_block(int fd, uint16_t size, uint64_t ptr, void* blk) {
    if(lseek(fd, ptr, SEEK_SET) < 0) return NULL;
    putle64(blk, ptr);
    putle16(blk+8, size);
    blk += 10;
    if(read(fd, blk, size) != size) return NULL;
    return blk;
}

int sync_block(int fd, void* blk) {
    uint64_t off = le64(blk-10);
    uint16_t size = le16(blk-2);
    if(size > PAGESZ) {
        errno = EFBIG;
        return EOF;
    }
    if(lseek(fd, off, SEEK_SET) < 0) return EOF;
    return write(fd, blk, size) != size;
}

int free_block(int fd, void* blk) {
    uint64_t ptr = 8, prev_ptr = 0, prev_prev_ptr = 0, off = le64(blk-10);
    uint8_t buf[8];
    uint16_t size = le16(blk-2), sz;
    while(ptr && ptr < off) {
        if(prev_ptr == ptr) return EOF;
        if(prev_prev_ptr && ptr < prev_ptr) { // 不符合顺序，进行一次调整
            lseek(fd, prev_prev_ptr, SEEK_SET);
            putle64(buf, ptr);
            if(write(fd, buf, 8) != 8) return EOF; // 1->next = 3
            if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
            prev_prev_ptr = ptr;                   // new1 = 3
            readle64(fd, ptr);                     // ptr(new3) = 3->next
            lseek(fd, -8, SEEK_SET);
            putle64(buf, prev_ptr);                // new2 = 2
            if(write(fd, buf, 8) != 8) return EOF; // 3->next = 2
            lseek(fd, prev_ptr, SEEK_SET);
            putle64(buf, ptr);
            if(write(fd, buf, 8) != 8) return EOF; // 2->next = ptr
            continue;
        }
        prev_prev_ptr = prev_ptr;
        prev_ptr = ptr;
        if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
        readle64(fd, ptr);
    }
    putle64(blk, ptr);
    putle16(blk+8, size);
    putle64(buf, off);
    lseek(fd, prev_ptr, SEEK_SET);
    if(write(fd, buf, 8) != 8) return EOF; // 将本页附加到链表
    readle16(fd, sz);
    if(prev_ptr+sz == off && prev_ptr%PAGESZ < off%PAGESZ) { // 可以和前一块合并
        lseek(fd, prev_ptr, SEEK_SET);
        write(fd, blk, 8);
        putle16(buf, size+sz);
        write(fd, buf, 2);
        return 0;
    }
    if(off+size == ptr && off%PAGESZ < ptr%PAGESZ) { // 可以和后一块合并
        if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
        readle64(fd, prev_ptr);
        readle16(fd, sz);
        putle64(blk, prev_ptr);
        putle16(blk, size+sz);
    }
    sync_block(fd, blk);
    return 0;
}

int add_block(int fd, uint16_t size, uint64_t off) {
    uint64_t ptr = 8, prev_ptr = 0, prev_prev_ptr = 0;
    uint8_t buf[8];
    uint16_t sz;
    while(ptr && ptr < off) {
        if(prev_ptr == ptr) return EOF;
        if(prev_prev_ptr && ptr < prev_ptr) { // 不符合顺序，进行一次调整
            lseek(fd, prev_prev_ptr, SEEK_SET);
            putle64(buf, ptr);
            if(write(fd, buf, 8) != 8) return EOF; // 1->next = 3
            if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
            prev_prev_ptr = ptr;                   // new1 = 3
            readle64(fd, ptr);                     // ptr(new3) = 3->next
            lseek(fd, -8, SEEK_SET);
            putle64(buf, prev_ptr);                // new2 = 2
            if(write(fd, buf, 8) != 8) return EOF; // 3->next = 2
            lseek(fd, prev_ptr, SEEK_SET);
            putle64(buf, ptr);
            if(write(fd, buf, 8) != 8) return EOF; // 2->next = ptr
            continue;
        }
        prev_prev_ptr = prev_ptr;
        prev_ptr = ptr;
        if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
        readle64(fd, ptr);
    }
    if(lseek(fd, off, SEEK_SET) < 0) return EOF;
    putle64(buf, ptr);
    write(fd, buf, 8);
    putle16(buf, size);
    write(fd, buf, 2);
    lseek(fd, prev_ptr, SEEK_SET);
    putle64(buf, off);
    if(write(fd, buf, 8) != 8) return EOF; // 将本页附加到链表
    readle16(fd, sz);
    if(prev_ptr+sz == off && prev_ptr%PAGESZ < off%PAGESZ) { // 可以和前一块合并
        lseek(fd, prev_ptr, SEEK_SET);
        putle64(buf, ptr);
        write(fd, buf, 8);
        putle16(buf, size+sz);
        write(fd, buf, 2);
        return 0;
    }
    if(off+size == ptr && off%PAGESZ < ptr%PAGESZ) { // 可以和后一块合并
        if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
        readle64(fd, prev_ptr);
        readle16(fd, sz);
        lseek(fd, off, SEEK_SET);
        putle64(buf, prev_ptr);
        write(fd, buf, 8);
        putle16(buf, size+sz);
        write(fd, buf, 2);
    }
    return 0;
}
