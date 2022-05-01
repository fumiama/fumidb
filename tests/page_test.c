#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/binary.h"
#include "../include/file.h"
#include "../include/page.h"

void* pages[16];
uint8_t nullpage[PAGESZ];

static uint64_t get_second_unused_block(int fd) {
    uint64_t ptr = get_first_unused_block(fd);
    if(!ptr) return 0;
    if(lseek(fd, ptr, SEEK_SET) < 0) return 0;
    readle64(fd, ptr);
    return ptr;
}

int main() {
    int fd = open("page_test_tmp.bin", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(fd < 0) {
        perror("create");
        return 1;
    }
    if(init_file_header_page(fd) < 0) return 2;
    for(int i = 0; i < 16; i++) {
        void* page = alloc_page(fd);
        if(page == NULL) {
            perror("alloc_page");
            return 3;
        }
        pages[i] = page;
    }
    puts("free!");
    free_page(fd, pages[15]);
    if(get_second_unused_block(fd) != 16*PAGESZ) return 5;
    puts("free 15!");
    free_page(fd, pages[12]);
    if(get_second_unused_block(fd) != 13*PAGESZ) return 6;
    puts("free 12!");
    free_page(fd, pages[1]);
    if(get_second_unused_block(fd) != 2*PAGESZ) return 7;
    puts("free 1!");
    free_page(fd, pages[10]);
    if(get_second_unused_block(fd) != 2*PAGESZ) return 8;
    puts("free 10!");
    free_page(fd, pages[9]);
    if(get_second_unused_block(fd) != 2*PAGESZ) return 8;
    puts("free 9!");
    pages[1] = alloc_page(fd);
    if(le64(pages[1]-8) != (uint64_t)(2*PAGESZ)) {
        printf("1: %016llx != %016llx\n", le64(pages[1]-8), (uint64_t)(2*PAGESZ));
        return 9;
    }
    pages[9] = alloc_page(fd);
    if(le64(pages[9]-8) != (uint64_t)(10*PAGESZ)) {
        printf("9: %016llx != %016llx\n", le64(pages[9]-8), (uint64_t)(11*PAGESZ));
        return 10;
    }
    pages[10] = alloc_page(fd);
    if(le64(pages[10]-8) != (uint64_t)(11*PAGESZ)) {
        printf("10: %016llx != %016llx\n", le64(pages[10]-8), (uint64_t)(11*PAGESZ));
        return 11;
    }
    pages[12] = alloc_page(fd);
    if(le64(pages[12]-8) != (uint64_t)(13*PAGESZ)) return 12;
    pages[15] = alloc_page(fd);
    if(le64(pages[15]-8) != (uint64_t)(16*PAGESZ)) return 13;
    for(int i = 0; i < 16; i++) {
        if(free_page(fd, pages[i])) {
            perror("free_page");
            return 14;
        }
    }
    close(fd);

    fd = open("page_test_tmp.bin", O_RDWR, 0644);
    if(fd < 0) {
        perror("open");
        return 15;
    }
    uint8_t* blk1 = alloc_block(fd, 40);
    uint8_t* blk2 = alloc_block(fd, 22);
    uint8_t* blk3 = alloc_block(fd, 33);
    uint8_t* blk4 = alloc_block(fd, 4095);
    memcpy(blk1, "hello world!", 13);
    sync_block(fd, blk1);
    lseek(fd, HEADERSZ, SEEK_SET);
    read(fd, blk2, 13);
    puts("hello world!");
    if(strcmp((const char *)blk2, (const char *)blk1)) {
        return 16;
    }
    puts("hello world 1!");
    sync_block(fd, blk2);
    lseek(fd, HEADERSZ+40, SEEK_SET);
    read(fd, blk3, 13);
    if(strcmp((const char *)blk3, (const char *)blk1)) {
        return 17;
    }
    puts("hello world 2!");
    sync_block(fd, blk3);
    lseek(fd, HEADERSZ+40+22, SEEK_SET);
    read(fd, blk4+222, 13);
    sync_block(fd, blk4);
    if(strcmp((const char *)&blk4[222], (const char *)blk1)) {
        return 18;
    }
    puts("hello world 3!");
    memset(blk1, 0, 40);
    lseek(fd, PAGESZ+222, SEEK_SET);
    read(fd, blk1, 13);
    if(strcmp((const char *)blk1, (const char *)blk2)) {
        return 19;
    }
    puts("hello world4!");
    if(free_block(fd, blk1)) {
        perror("free_block(fd, blk1)");
        return 20;
    }
    if(free_block(fd, blk2)) {
        perror("free_block(fd, blk2)");
        return 21;
    }
    if(free_block(fd, blk3)) {
        perror("free_block(fd, blk3)");
        return 22;
    }
    if(free_block(fd, blk4)) {
        perror("free_block(fd, blk4)");
        return 23;
    }
    blk1 = alloc_block(fd, 40+22+33);
    memcpy(blk1+44, "hello world!", 13);
    sync_block(fd, blk1);
    lseek(fd, HEADERSZ+44, SEEK_SET);
    char buf[13];
    read(fd, buf, 13);
    if(strcmp((const char *)&blk1[44], (const char *)buf)) {
        return 24;
    }
    close(fd);

    fd = open("page_test_tmp.bin", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(fd < 0) {
        perror("create");
        return 25;
    }
    if(init_file_header_page(fd) < 0) return 26;
    blk1 = alloc_block(fd, 40);
    blk2 = alloc_block(fd, 22);
    blk3 = alloc_block(fd, 33);
    blk4 = alloc_block(fd, 4095);
    memcpy(blk1, "hello world!", 13);
    sync_block(fd, blk1);
    lseek(fd, HEADERSZ, SEEK_SET);
    read(fd, blk2, 13);
    if(strcmp((const char *)blk2, (const char *)blk1)) {
        return 16;
    }
    sync_block(fd, blk2);
    lseek(fd, HEADERSZ+40, SEEK_SET);
    read(fd, blk3, 13);
    if(strcmp((const char *)blk3, (const char *)blk1)) {
        return 17;
    }
    sync_block(fd, blk3);
    lseek(fd, HEADERSZ+40+22, SEEK_SET);
    read(fd, blk4+222, 13);
    sync_block(fd, blk4);
    if(strcmp((const char *)&blk4[222], (const char *)blk1)) {
        return 18;
    }
    memset(blk1, 0, 40);
    lseek(fd, PAGESZ+222, SEEK_SET);
    read(fd, blk1, 13);
    if(strcmp((const char *)blk1, (const char *)blk2)) {
        return 19;
    }
    if(free_block(fd, blk1)) {
        perror("free_block(fd, blk1)");
        return 20;
    }
    if(free_block(fd, blk2)) {
        perror("free_block(fd, blk2)");
        return 21;
    }
    if(free_block(fd, blk3)) {
        perror("free_block(fd, blk3)");
        return 22;
    }
    if(free_block(fd, blk4)) {
        perror("free_block(fd, blk4)");
        return 23;
    }
    blk1 = alloc_block(fd, 40+22+33);
    memcpy(blk1+44, "hello world!", 13);
    sync_block(fd, blk1);
    lseek(fd, HEADERSZ+44, SEEK_SET);
    memset(buf, 0, 13);
    read(fd, buf, 13);
    if(strcmp((const char *)&blk1[44], (const char *)buf)) {
        return 24;
    }
    close(fd);

    remove("page_test_tmp.bin");
    return 0;
}
