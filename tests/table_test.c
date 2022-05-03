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
        fd, buf, "test_table", 5,
        TYPE_INT16|EXTYPE_NONNULL|EXTYPE_UNIQUE,
        TYPE_INT64,
        TYPE_INT8,
        TYPE_STRING,
        TYPE_BINARY
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
    close(fd);
}
