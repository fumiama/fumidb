#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

#include "../include/binary.h"
#include "../include/page.h"
#include "../include/file.h"

uint8_t header[PAGESZ] = {'F', 'U', 'M', 'I', 'D', 'B', 1, 0};

int init_file_header_page(int fd) {
    lseek(fd, 0, SEEK_SET);
    return write(fd, header, PAGESZ) != PAGESZ;
}

uint16_t get_db_version(int fd) {
    uint16_t v;
    if(lseek(fd, 6, SEEK_SET) < 0) return EOF;
    readle16(fd, v);
    return v;
}

int set_first_unused_block(int fd, uint64_t ptr) {
    uint8_t buf[8];
    if(lseek(fd, 8, SEEK_SET) < 0) return EOF;
    putle64(buf, ptr);
    return write(fd, buf, 8) != 8;
}

uint64_t get_first_unused_block(int fd) {
    uint64_t ptr;
    if(lseek(fd, 8, SEEK_SET) < 0) return EOF;
    readle64(fd, ptr);
    return ptr;
}

int set_first_table(int fd, uint64_t ptr) {
    uint8_t buf[8];
    if(lseek(fd, 16, SEEK_SET) < 0) return EOF;
    putle64(buf, ptr);
    return write(fd, buf, 8) != 8;
}

uint64_t get_first_table(int fd) {
    uint64_t ptr;
    if(lseek(fd, 16, SEEK_SET) < 0) return EOF;
    readle64(fd, ptr);
    return ptr;
}
