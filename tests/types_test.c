#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/binary.h"
#include "../include/file.h"
#include "../include/page.h"
#include "../include/types.h"
#include "../include/types/int8.h"

uint8_t int8buf[INT8_INDEX_SZ+10];

int main() {
    /*                             test int8                             */
    int fd = open("types_test_tmp.bin", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(fd < 0) {
        perror("create");
        return 1;
    }
    if(init_file_header_page(fd) < 0) return 2;
    void* index = create_index(fd, TYPE_INT8, int8buf);
    if(!index) {
        perror("create_index");
        return 3;
    }
    if(le64(index-10) != HEADERSZ) {
        printf("%016llx\n", le64(index-10));
        return 4;
    }
    if(le16(index-2) != INT8_INDEX_SZ) {
        printf("%04x\n", le16(index-2));
        return 5;
    }
    insert_item(fd, TYPE_INT8, index, 1, 3456432);
    insert_item(fd, TYPE_INT8, index, 3, 7654323456);
    insert_item(fd, TYPE_INT8, index, 45, 345743415);
    insert_item(fd, TYPE_INT8, index, 67, 56787145);
    insert_item(fd, TYPE_INT8, index, 123, 123567854424);
    if(find_item_by_key(fd, TYPE_INT8, index, 1) != 3456432) return 6;
    if(find_item_by_key(fd, TYPE_INT8, index, 3) != 7654323456) return 7;
    if(find_item_by_key(fd, TYPE_INT8, index, 45) != 345743415) return 8;
    if(find_item_by_key(fd, TYPE_INT8, index, 67) != 56787145) return 9;
    if(find_item_by_key(fd, TYPE_INT8, index, 123) != 123567854424) return 10;
    if(find_item_by_key(fd, TYPE_INT8, index, 255) != 0) return 11;
    index = NULL;
    close(fd);
    fd = open("types_test_tmp.bin", O_RDWR, 0644);
    memset(int8buf, 0, INT8_INDEX_SZ+10);
    index = load_index(fd, TYPE_INT8, HEADERSZ, int8buf);
    if(find_item_by_key(fd, TYPE_INT8, index, 1) != 3456432) return 6;
    if(find_item_by_key(fd, TYPE_INT8, index, 3) != 7654323456) return 7;
    if(find_item_by_key(fd, TYPE_INT8, index, 45) != 345743415) return 8;
    if(find_item_by_key(fd, TYPE_INT8, index, 67) != 56787145) return 9;
    if(find_item_by_key(fd, TYPE_INT8, index, 123) != 123567854424) return 10;
    if(find_item_by_key(fd, TYPE_INT8, index, 255) != 0) return 11;
    remove_item_by_key(fd, TYPE_INT8, index, 123);
    if(find_item_by_key(fd, TYPE_INT8, index, 123) != 0) return 12;
    close(fd);
    /*                         end test int8                             */
    // remove("types_test_tmp.bin");
}
