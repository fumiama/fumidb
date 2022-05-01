#include <unistd.h>
#include <fcntl.h>
#include "../include/binary.h"
#include "../include/file.h"
#include "../include/types.h"

int main() {
    int fd = open("types_test_tmp.bin", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(fd < 0) {
        perror("create");
        return 1;
    }
    if(init_file_header_page(fd) < 0) return 2;
    uint64_t ptr = create_index(fd, TYPE_INT8);
    if(!ptr) {
        perror("create_index");
        return 3;
    }
    close(fd);
    // remove("types_test_tmp.bin");
}
