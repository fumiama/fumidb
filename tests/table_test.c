#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/binary.h"
#include "../include/file.h"
#include "../include/page.h"
#include "../include/table.h"
#include "../include/types.h"

char buf[4106];

int main() {
    int fd = open("table_test_tmp.bin", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(fd < 0) {
        perror("create");
        return 1;
    }
    if(init_file_header_page(fd)) return 2;
    void* table = create_table(
        fd, buf, "test_table", 6, (type_t[]){
            TYPE_INT16|EXTYPE_NONNULL|EXTYPE_UNIQUE,
            TYPE_INT64,
            TYPE_INT8,
            TYPE_INT32,
            TYPE_STRING,
            TYPE_BINARY
        }
    );
    if(table == NULL) {
        perror("create_table");
        return 3;
    }
    int len = get_table_name_length(table);
    if(len != sizeof("test_table")) return 4;
    char namebuf[len];
    if(strcmp(get_table_name(table, namebuf), "test_table")) return 5;
    close(fd);
    fd = open("table_test_tmp.bin", O_RDWR, 0644);
    if(fd < 0) {
        perror("open");
        return 1;
    }
    memset(buf, 0, sizeof(buf));
    table = load_table(fd, buf, get_first_table(fd));
    if(table == NULL) {
        perror("load_table");
        return 3;
    }
    len = get_table_name_length(table);
    if(len != sizeof("test_table")) {
        printf("%d\n", len);
        return 4;
    }
    if(strcmp(get_table_name(table, namebuf), "test_table")) return 5;
    if(get_index_ptr(table, 0) != 0x100) return 6;
    if(insert_row(fd, table, (key_t[]){
        2333,
        EXTYPE_NONNULL, 234532121345,
        EXTYPE_NONNULL, 88,
        EXTYPE_NULL,
        EXTYPE_NONNULL, 12, (key_t)"test_string",
        EXTYPE_NONNULL, 5, (key_t)"\x45\xff\x12\xda"
    }) == 0) {
        perror("insert_row");
        return 7;
    }
    uint64_t ptr = find_row_by_pk(fd, table, 2333);
    if(ptr == 0) {
        perror("find_row_by_pk");
        return 8;
    }

    if(lseek(fd, ptr, SEEK_SET) < 0) return 9;
    readle16(fd, ptr);
    if((uint16_t)ptr != 2333) {
        printf("pk: %d\n", (uint16_t)ptr);
        return 10;
    }

    char isnull = 1;
    read(fd, &isnull, 1);
    if(isnull) return 11;
    readle64(fd, ptr);
    if(ptr != 234532121345) return 12;

    isnull = 1;
    read(fd, &isnull, 1);
    if(isnull) return 11;
    lseek(fd, 8, SEEK_CUR);
    read(fd, &isnull, 1);
    if(isnull != 88) return 13;

    isnull = 0;
    read(fd, &isnull, 1);
    if(!isnull) return 11;
    lseek(fd, 4, SEEK_CUR);

    isnull = 1;
    read(fd, &isnull, 1);
    if(isnull) return 11;
    lseek(fd, 8, SEEK_CUR);
    readle16(fd, ptr);
    if((uint16_t)ptr != 12) {
        printf("string len: %d\n", (uint16_t)ptr);
        return 10;
    }
    char buf[20];
    read(fd, buf, 12);
    if(strcmp(buf, "test_string")) return 14;

    isnull = 1;
    read(fd, &isnull, 1);
    if(isnull) return 11;
    readle16(fd, ptr);
    if((uint16_t)ptr != 5) {
        printf("binary len: %d\n", (uint16_t)ptr);
        return 10;
    }
    read(fd, buf, 5);
    if(strcmp(buf, "\x45\xff\x12\xda")) return 15;
    close(fd);
    // remove("table_test_tmp.bin");
}
