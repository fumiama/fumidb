#include <unistd.h>
#include <fcntl.h>
#include "../include/binary.h"

int main() {
    unsigned char buf[8];
    putle16(buf, 1);
    if(buf[0] != 1 || le16(buf) != 0x01) return 1;
    putle32(buf+1, 2);
    if(buf[1] != 2 || le32(buf+1) != 0x02) return 2;
    putle64(buf, 0xff0000);
    if(buf[2] != 0xff || le64(buf) != 0xff0000) return 3;

    int fd = open("binary_test_tmp.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd < 0) {
        perror("create");
        return 4;
    }
    write(fd, "\001\002\003\004\005\006\a\b", 8);
    close(fd);
    fd = open("binary_test_tmp.bin", O_RDONLY | O_NONBLOCK);
    if(fd < 0) {
        perror("open");
        return 5;
    }
    uint64_t ptr = 0;
    readle16(fd, ptr);
    printf("%04x\n", (uint16_t)ptr);
    if((uint16_t)ptr != 0x0201) return 6;
    lseek(fd, 0, SEEK_SET);
    readle32(fd, ptr);
    printf("%08x\n", (uint32_t)ptr);
    if((uint32_t)ptr != 0x04030201) return 7;
    lseek(fd, 0, SEEK_SET);
    readle64(fd, ptr);
    printf("%016llx\n", (uint64_t)ptr);
    if((uint64_t)ptr != 0x0807060504030201) return 8;
    close(fd);
    remove("binary_test_tmp.bin");
    return 0;
}
