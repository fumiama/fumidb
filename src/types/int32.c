#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../include/binary.h"
#include "../../include/page.h"
#include "../../include/types/int32.h"

void* create_int32_index(int fd, void* buf) {
    void* page = alloc_page(fd, buf);
    if(unlikely(page == NULL)) return NULL;
    memset(page, 0, PAGESZ);
    void* prev_page = page;
    for(int i = 1; i < INT32_BUCKET_SZ; i++) {
        page = alloc_page(fd, buf);
        if(unlikely(page == NULL)) {
            for(int j = 0; j < i; j++) free_page(fd, buf+j*(PAGESZ+8)+8);
            return NULL;
        }
        memset(page, 0, PAGESZ);
        putle64(prev_page+PAGESZ-8, le64(page-8));
        if(unlikely(sync_page(fd, prev_page))) {
            free_page(fd, page);
            for(int j = 0; j < i; j++) free_page(fd, buf+j*(PAGESZ+8)+8);
            return NULL;
        }
        prev_page = page;
    }
    if(unlikely(sync_page(fd, page))) {
        free_page(fd, page);
        for(int i = 0; i < INT32_BUCKET_SZ; i++) free_page(fd, buf+i*(PAGESZ+8)+8);
        return NULL;
    }
    return buf+8;
}

void* load_int32_index(int fd, uint64_t ptr, void* buf) {
    void* page = get_page(fd, ptr, buf);
    if(unlikely(page == NULL)) return NULL;
    for(int i = 1; i < INT32_BUCKET_SZ-1; i++) {
        page = get_page(fd, le64(page+PAGESZ-8), buf+i*(PAGESZ+8));
        if(unlikely(page == NULL)) return NULL;
    }
    return buf+8;
}

int remove_int32_index(int fd, void* index) {
    for(int i = 0; i < INT32_BUCKET_SZ; i++) {
        void* page = index+i*(PAGESZ+8);
        uint64_t ptr = le64(page+PAGESZ-16);
        while(unlikely(ptr)) {
            void* p = get_page(fd, ptr, index+INT32_INDEX_SZ-BUFSIZ-8);
            if(unlikely(p == NULL)) return EOF;
            ptr = le64(p+PAGESZ-16);
            if(unlikely(free_page(fd, p))) return EOF;
        }
        if(unlikely(free_page(fd, page))) return EOF;
    }
}

uint64_t count_int32_items(int fd, void* index) {
    
}

int insert_int32_item(int fd, void* index, key_t k, uint64_t ptr) {
    
}

uint64_t find_item_by_int32_key(int fd, void* index, key_t k) {
    
}

uint64_t remove_item_by_int32_key(int fd, void* index, key_t k) {
    
}
