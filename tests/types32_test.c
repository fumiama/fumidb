#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/binary.h"
#include "../include/file.h"
#include "../include/page.h"
#include "../include/types.h"
#include "../include/types/int32.h"

uint8_t buf[INT32_INDEX_SZ];

int main() {
    /*                             test int32                             */
    int fd = open("types_test_tmp.bin", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(fd < 0) {
        perror("create");
        return 1;
    }
    if(init_file_header_page(fd) < 0) return 2;
    void* index = create_index(fd, TYPE_INT32, buf);
    if(!index) {
        perror("create_int32_index");
        return 3;
    }
    for(int i = 0, j = 8192; i < INT32_BUCKET_SZ-1; i++, j+=PAGESZ) {
        if(le64(index+i*(PAGESZ+8)+PAGESZ-8) != j) {
            printf("index: %d, ptr: %d!=%d\n", i, le64(index+PAGESZ-8), j);
            return 3;
        }
    }
    if(le64(index+255*(PAGESZ+8)+PAGESZ-8) != 0) {
        printf("index: 255, ptr: %d!=%d\n", le64(index+PAGESZ-8), 0);
        return 3;
    }
    int cnt = 0;
    for(int i = 123456; i < 123456+8192; i++, cnt++) {
        int n;
        if((n=count_items(fd, TYPE_INT32, index)) != cnt) {
            printf("%d != %d\n", cnt, n);
            return 4;
        }
        // printf("c", i);
        if(insert_item(fd, TYPE_INT32, index, (key_t)i, i)) {
            printf("%u ", (uint32_t)i);
            fflush(stdout);
            perror("insert_int32_item");
            return 4;
        }
        // printf("i ", i);
    }

    for(int i = 123456; i < 123456+8192; i++) {
        if((int)find_item_by_key(fd, TYPE_INT32, index, (key_t)i) != i) {
            printf("%u ", (uint32_t)i);
            fflush(stdout);
            perror("find_item_by_key");
            return 5;
        }
    }
    
    close(fd);
    fd = open("types_test_tmp.bin", O_RDWR, 0644);
    memset(buf, 0, sizeof(buf));
    index = load_index(fd, TYPE_INT32, PAGESZ, buf);

    for(int i = 0, j = 8192; i < INT32_BUCKET_SZ-1; i++, j+=PAGESZ) {
        if(le64(index+i*(PAGESZ+8)+PAGESZ-8) != j) {
            printf("index: %d, ptr: %d!=%d\n", i, le64(index+PAGESZ-8), j);
            return 3;
        }
    }

    if(le64(index+255*(PAGESZ+8)+PAGESZ-8) != 0) {
        printf("index: 255, ptr: %d!=%d\n", le64(index+PAGESZ-8), 0);
        return 3;
    }

    for(int i = 123456; i < 123456+4098; i++, cnt--) {
        int n;
        if((n=count_items(fd, TYPE_INT32, index)) != cnt) {
            printf("%d != %d\n", cnt, n);
            return 6;
        }
        if(remove_item_by_key(fd, TYPE_INT32, index, (key_t)i) != i) {
            printf("%u ", (uint32_t)i);
            fflush(stdout);
            perror("remove_item_by_key");
            return 6;
        }
    }

    for(int i = 123456; i < 123456+4098; i++) {
        if((int)find_item_by_key(fd, TYPE_INT32, index, (key_t)i) != 0) {
            printf("%u ", (uint32_t)i);
            fflush(stdout);
            perror("find_item_by_key");
            return 7;
        }
    }

    for(int i = 123456+4098; i < 123456+8192; i++) {
        if((int)find_item_by_key(fd, TYPE_INT32, index, (key_t)i) != i) {
            printf("%u ", (uint32_t)i);
            fflush(stdout);
            perror("find_item_by_key");
            return 8;
        }
    }

    close(fd);
    /*                         end test int32                             */
    // remove("types_test_tmp.bin");
    return 0;
}
