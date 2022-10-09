#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../include/binary.h"
#include "../../include/page.h"
#include "../../include/types/int64.h"

//#define DEBUG

void* create_int64_index(int fd, void* buf) {
    void* page = alloc_page(fd, buf);
    if(unlikely(page == NULL)) return NULL;
    memset(page, 0, PAGESZ);
    void* prev_page = page;
    for(int i = 1; i < INT64_BUCKET_SZ; i++) {
        page = alloc_page(fd, buf+i*(PAGESZ+8));
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
        for(int i = 0; i < INT64_BUCKET_SZ; i++) free_page(fd, buf+i*(PAGESZ+8)+8);
        return NULL;
    }
    #ifdef DEBUG
        puts("alloc finish");
    #endif
    return buf+8;
}

void* load_int64_index(int fd, uint64_t ptr, void* buf) {
    void* page = get_page(fd, ptr, buf);
    if(unlikely(page == NULL)) return NULL;
    for(int i = 1; i < INT64_BUCKET_SZ; i++) {
        page = get_page(fd, le64(page+PAGESZ-8), buf+i*(PAGESZ+8));
        if(unlikely(page == NULL)) return NULL;
    }
    return buf+8;
}

int remove_int64_index(int fd, void* index) {
    for(int i = 0; i < INT64_BUCKET_SZ; i++) {
        void* page = index+i*(PAGESZ+8);
        uint64_t ptr = le64(page+PAGESZ-16);
        while(unlikely(ptr)) {
            void* p = get_page(fd, ptr, index-8+INT64_INDEX_SZ-PAGESZ-8);
            if(unlikely(p == NULL)) return EOF;
            ptr = le64(p+PAGESZ-16);
            if(unlikely(free_page(fd, p))) return EOF;
        }
        if(unlikely(free_page(fd, page))) return EOF;
    }
    return 0;
}

uint64_t count_int64_items(int fd, void* index) {
    uint64_t sum = 0;
    for(int i = 0; i < INT64_BUCKET_SZ; i++) {
        void* page = index+i*(PAGESZ+8);
        uint64_t ptr = 1;
        for(int j = 0; j < INT64_BUCKET_MAX_ITEM_SZ && ptr; j++) {
            ptr = le64(page+j*(8+8));
            sum += !!ptr;
        }
        ptr = le64(page+PAGESZ-16); // 查看是否有溢出桶
        while(ptr) {
            #ifdef DEBUG
                //printf("%x ", ptr);
                //fflush(stdout);
            #endif
            page = get_page(fd, ptr, index-8+INT64_INDEX_SZ-PAGESZ-8);
            for(int j = 0; j < INT64_BUCKET_MAX_ITEM_SZ && ptr; j++) {
                ptr = le64(page+j*(8+8));
                sum += !!ptr;
            }
            ptr = le64(page+PAGESZ-16); // 查看是否有溢出桶
        }
    }
    return sum;
}

int insert_int64_item(int fd, void* index, key_t k, uint64_t ptr) {
    uint64_t key = (uint64_t)k;
    uint64_t digest = (k*(k+INT64_DIGEST_FACTOR))%256;
    // 访问digest指向的存储桶
    void* page = index+digest*(PAGESZ+8);
    uint64_t p = 1;
    while(1) {
        for(int j = 0; j < INT64_BUCKET_MAX_ITEM_SZ && p; j++) {
            void* slot = page+j*(8+8);
            p = le64(slot);
            if(p && key == le64(slot+8)) { // 已存在值, 替换之
                putle64(slot, ptr);
                return sync_page(fd, page);
            } else if(!p) { // 不存在值, 新建
                putle64(slot, ptr);
                putle64(slot+8, key);
                return sync_page(fd, page);
            }
        }
        // 溢出
        p = le64(page+PAGESZ-16); // 查看是否有溢出桶
        if(!p) { // 无溢出桶, 新建
            void* ovpage = alloc_page(fd, index-8+INT64_INDEX_SZ-PAGESZ-8);
            if(unlikely(ovpage == NULL)) return EOF;
            memset(ovpage, 0, PAGESZ);
            putle64(page+PAGESZ-16, le64(ovpage-8));
            #ifdef DEBUG
                printf("new ovpage: %x, this page: %x\n", le64(page+PAGESZ-16), le64(page-8));
            #endif
            int r = sync_page(fd, page);
            if(unlikely(r)) return r;
            putle64(ovpage, ptr);
            putle64(ovpage+8, key);
            #ifdef DEBUG
                printf("write ptr: %x, key: %x\n", ptr, key);
            #endif
            return sync_page(fd, ovpage);
        }
        // 有溢出桶, 进入下一轮搜索
        page = get_page(fd, p, index-8+INT64_INDEX_SZ-PAGESZ-8);
    }
}

uint64_t find_item_by_int64_key(int fd, void* index, key_t k) {
    uint64_t key = (uint64_t)k;
    uint64_t digest = (k*(k+INT64_DIGEST_FACTOR))%256;
    // 访问digest指向的存储桶
    void* page = index+digest*(PAGESZ+8);
    uint64_t p = 1;
    while(1) {
        for(int j = 0; j < INT64_BUCKET_MAX_ITEM_SZ && p; j++) {
            void* slot = page+j*(8+8);
            p = le64(slot);
            if(!p || (p && key == le64(slot+8))) { // 找到项目 or 不存在值, 无该项目
                return p;
            }
        }
        // 桶已满
        p = le64(page+PAGESZ-16); // 查看是否有溢出桶
        if(!p) { // 无溢出桶, 无该项目
            return 0;
        }
        // 有溢出桶, 进入下一轮搜索
        page = get_page(fd, p, index-8+INT64_INDEX_SZ-PAGESZ-8);
    }
}

uint64_t remove_item_by_int64_key(int fd, void* index, key_t k) {
    uint64_t key = (uint64_t)k;
    uint64_t digest = (k*(k+INT64_DIGEST_FACTOR))%256;
    // 访问digest指向的存储桶
    void* page = index+digest*(PAGESZ+8);
    uint64_t p = 1;
    while(1) {
        for(int j = 0; j < INT64_BUCKET_MAX_ITEM_SZ && p; j++) {
            void* slot = page+j*(8+8);
            p = le64(slot);
            if(!p) { // 不存在值, 无该项目
                return 0;
            }
            if(p && key == le64(slot+8)) { // 找到项
                int i = j;
                for(; i < INT64_BUCKET_MAX_ITEM_SZ && le64(page+i*(8+8)); i++) {
                    #ifdef DEBUG
                        //printf("lookup %03d: %x\n", i, le64(page+i*(8+8)));
                    #endif
                }
                #ifdef DEBUG
                    printf("page: %x(%x), i: %d, p: %x\n", page, le64(page-8), i, p);
                #endif
                void *next_page = page, *prev_page;
                while(i == INT64_BUCKET_MAX_ITEM_SZ) { // 本桶已满
                    int next = le64(next_page+PAGESZ-16); // 查看是否有溢出桶
                    if(!next) { // 无溢出桶, 将最后一个值搬移至slot并为其补0
                        // 不是最后一项, 搬移
                        if(j+1 != INT64_BUCKET_MAX_ITEM_SZ || next_page!=page) memcpy(slot, next_page-8-8-8-8, 8+8);
                        memset(next_page-8-8-8-8, 0, 8+8);
                        int r = sync_page(fd, next_page);
                        if(unlikely(r)) return r;
                        if(next_page!=page) {
                            r = sync_page(fd, page);
                            if(unlikely(r)) return r;
                        }
                        return p;
                    }
                    // 有溢出桶, 加载
                    prev_page = next_page;
                    #ifdef DEBUG
                        printf("%x have next bucket at: %x, ", le64(prev_page-8), next);
                    #endif
                    void* buf = index-8+INT64_INDEX_SZ-PAGESZ-8-PAGESZ-8;
                    #ifdef DEBUG
                        printf("buf: %p, ", buf);
                        fflush(stdout);
                    #endif
                    next_page = get_page(fd, next, buf);
                    #ifdef DEBUG
                        printf("next page: %p\n", next_page);
                    #endif
                    for(i = 0; i < INT64_BUCKET_MAX_ITEM_SZ && le64(next_page+i*(8+8)); i++);
                    #ifdef DEBUG
                        printf("new i: %d\n", i);
                    #endif
                }
                // 转至新桶第一个, 但发现为空, 即最后一项在上个桶末尾
                if(!i) {
                    #ifdef DEBUG
                        puts("i == 0");
                    #endif
                    memcpy(slot, prev_page+PAGESZ-8-8-8-8, 8+8);
                    memset(prev_page+PAGESZ-8-8-8-8, 0, 8+8);
                    #ifdef DEBUG
                        printf("prev_page: %x(%x), page: %x(%x)\n", prev_page, le64(prev_page-8), page, le64(page-8));
                        printf("slot@%016x+%03d, last@%016x+339, ptr: %x, key: %x\n", le64(page-8), (slot-page)/12, le64(prev_page-8), le64(slot), le64(slot+8));
                    #endif
                    int r = sync_page(fd, prev_page);
                    if(unlikely(r)) return r;
                    if(prev_page!=page) {
                        r = sync_page(fd, page);
                        if(unlikely(r)) return r;
                    }
                    return p;
                }
                // 本桶未满
                if(--i == j && page == next_page) { // 是最后一项, 直接删除
                    memset(slot, 0, 8+8);
                    int r = sync_page(fd, page);
                    if(unlikely(r)) return r;
                    return p;
                }
                // 不是最后一项, 搬移
                memcpy(slot, next_page+i*(8+8), 8+8);
                memset(next_page+i*(8+8), 0, 8+8);
                int r = sync_page(fd, next_page);
                if(unlikely(r)) return r;
                if(next_page!=page) {
                    r = sync_page(fd, page);
                    if(unlikely(r)) return r;
                }
                #ifdef DEBUG
                    printf("slot@%016x+%03d, last@%016x+%03d\n", le64(page-8), (slot-page)/12, le64(next_page-8), i);
                #endif
                return p;
            }
        }
        // 桶已满
        p = le64(page+PAGESZ-16); // 查看是否有溢出桶
        if(!p) { // 无溢出桶, 无该项目
            return 0;
        }
        // 有溢出桶, 进入下一轮搜索
        page = get_page(fd, p, index-8+INT64_INDEX_SZ-PAGESZ-8);
    }
}
