#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "../../include/binary.h"
#include "../../include/page.h"
#include "../../include/types/int16.h"

// len(buf) >= INT16_INDEX_SZ+10 + INT16_BITMAP_SZ+8*2 = 10290
// &buf[0]    ~ &buf[2081] is index, index = buf+10
// &buf[2082] ~ &buf[6185] is the first page of bitmap, ptr = buf+2090
// &buf[6186] ~ &buf[10289] is the second page of bitmap, ptr = buf+6194
// 返回：index = buf+10
void* create_int16_index(int fd, void* buf) {
    buf = alloc_block(fd, INT16_INDEX_SZ, buf);
    if(buf == NULL) return NULL;
    memset(buf, 0, INT16_INDEX_SZ);
    
    void* page = alloc_page(fd, buf+INT16_INDEX_SZ);
    if(page == NULL) {
        free_block(fd, buf);
        return NULL;
    }
    memset(page, 0, PAGESZ);
    sync_page(fd, page);

    void* page2 = alloc_page(fd, page+PAGESZ);
    if(page2 == NULL) {
        free_block(fd, buf);
        free_page(fd, page);
        return NULL;
    }
    memset(page2, 0, PAGESZ);
    sync_page(fd, page2);

    putle64(buf, le64(page-8));
    putle64(buf+8, le64(page2-8));
    sync_block(fd, buf);
    return buf;
}

// len(buf) >= INT16_INDEX_SZ+10 + INT16_BITMAP_SZ+8*2 = 10290
// &buf[0]    ~ &buf[2081] is index, index = buf+10
// &buf[2082] ~ &buf[6185] is the first page of bitmap, ptr = buf+2090
// &buf[6186] ~ &buf[10289] is the second page of bitmap, ptr = buf+6194
// 返回：index = buf+10
void* load_int16_index(int fd, uint64_t ptr, void* buf) {
    buf = get_block(fd, INT16_INDEX_SZ, ptr, buf);
    if(get_page(fd, le64(buf+8), get_page(fd, le64(buf), buf+INT16_INDEX_SZ)+PAGESZ) == NULL) return NULL;
    return buf;
}

int remove_int16_index(int fd, void* index) {
    uint64_t ptr = le64(index+16); // 链表头
    while(ptr) {
        uint64_t tmp;
        if(unlikely(lseek(fd, ptr, SEEK_SET) < 0)) return EOF; 
        readle64(fd, tmp);
        add_block(fd, INT16_CHAIN_SZ, ptr);
        ptr = tmp;
    }
    if(free_page(fd, index+INT16_INDEX_SZ+8)) return 2; // 第一页位图
    if(free_page(fd, index+INT16_INDEX_SZ+8+PAGESZ+8)) return 3; // 第二页位图
    return free_block(fd, index); // 位图索引
}

uint64_t count_int16_items(int fd, void* index) {
    int total = 0;
    // 计算总的条目数
    for(int i = 0; i < 128; i++) {
        int s = ((uint8_t*)(index+24))[i];
        if(!s && ((uint8_t*)(index+INT16_INDEX_SZ+8))[i*32]) {
            total += 256;
            continue;
        }
        total += s;
    }
    for(int i = 128; i < 256; i++) {
        int s = ((uint8_t*)(index+24))[i];
        if(!s && ((uint8_t*)(index+INT16_INDEX_SZ+8+8))[i*32]) {
            total += 256;
            continue;
        }
        total += s;
    }
    return total;
}

#define set_bitmap(fd, index, key) {\
    /* 写入位图 */\
    if(key<32768) {\
        ((uint8_t*)(index+INT16_INDEX_SZ+8))[key/8] |= 128>>(key%8);\
        if(unlikely(sync_page(fd, index+INT16_INDEX_SZ+8))) { /* 失败，撤销更改 */\
            ((uint8_t*)(index+INT16_INDEX_SZ+8))[key/8] &= ~(128>>(key%8));\
            return EOF;\
        }\
    } else {\
        ((uint8_t*)(index+INT16_INDEX_SZ+8+8))[key/8] |= 128>>(key%8);\
        if(unlikely(sync_page(fd, index+INT16_INDEX_SZ+8+PAGESZ+8))) { /* 失败，撤销更改 */\
            ((uint8_t*)(index+INT16_INDEX_SZ+8+8))[key/8] &= ~(128>>(key%8));\
            return EOF;\
        }\
    }\
    ((uint8_t*)(index+24))[key/256]++; /* 写入位图索引 */\
    if(unlikely(sync_block(fd, index))) { /* 失败，撤销更改 */\
        ((uint8_t*)(index+24))[key/256]--;\
        return EOF;\
    }\
}

int insert_int16_item(int fd, void* index, key_t k, uint64_t ptr) {
    int isexist, sum = 0, total = count_int16_items(fd, index);
    uint16_t key = (uint16_t)k;
    char buf[8];
    char tmp[10+INT16_CHAIN_SZ];
    putle64(buf, ptr);

    #ifdef DEBUG
        printf("No.%u: ", (int)key);
    #endif

    if(key < 32768) {
        // key是否已存在
        isexist = ((uint8_t*)(index+INT16_INDEX_SZ+8))[key/8] & (128>>(key%8));
        // 查找 key 之前共有多少索引
        for(int i = key/256*32; i < key/8; i++) { // 从未计算的32位组开始算起
            sum += __builtin_popcount(((uint8_t*)(index+INT16_INDEX_SZ+8))[i]);
        }
        sum += __builtin_popcount(((uint8_t*)(index+INT16_INDEX_SZ+8))[key/8] & ~(0xff>>(key%8)));
        #ifdef DEBUG
            printf("popc: %d, ", sum);
        #endif
        for(int i = 0; i < key/256; i++) {
            int s = ((uint8_t*)(index+24))[i];
            if(!s && ((uint8_t*)(index+INT16_INDEX_SZ+8))[i*32]) {
                sum += 256;
                continue;
            }
            sum += s;
        }
        #ifdef DEBUG
            printf("sum: %d, totl: %d, ", sum, total);
        #endif
    } else {
        // key是否已存在
        isexist = ((uint8_t*)(index+INT16_INDEX_SZ+8+8))[key/8] & (128>>(key%8));
        // 查找 key 之前共有多少索引
        for(int i = key/256*32; i < key/8; i++) { // 从未计算的32位组开始算起
            sum += __builtin_popcount(((uint8_t*)(index+INT16_INDEX_SZ+8+8))[i]);
        }
        sum += __builtin_popcount(((uint8_t*)(index+INT16_INDEX_SZ+8+8))[key/8] & ~(0xff>>(key%8)));
        #ifdef DEBUG
            printf("popc: %d, ", sum);
        #endif
        for(int i = 0; i < 128; i++) {
            int s = ((uint8_t*)(index+24))[i];
            if(!s && ((uint8_t*)(index+INT16_INDEX_SZ+8))[i*32]) {
                sum += 256;
                continue;
            }
            sum += s;
        }
        for(int i = 128; i < key/256; i++) {
            int s = ((uint8_t*)(index+24))[i];
            if(unlikely(!s && ((uint8_t*)(index+INT16_INDEX_SZ+8+8))[i*32])) {
                sum += 256;
                continue;
            }
            sum += s;
        }
        #ifdef DEBUG
            printf("sum: %d, totl: %d, ", sum, total);
        #endif
    }

    if(!isexist) {
        set_bitmap(fd, index, key);
        #ifdef DEBUG
            printf("i: %d, sumblk: %d, map: %02x, ", key/256, ((uint8_t*)(index+24))[key/256], ((uint8_t*)(index+INT16_INDEX_SZ+8+((key<32768)?0:8)))[key/8]);
        #endif
    }

    if(unlikely(!le64(index+16))) { // 插入的是本索引的第一个值
        if(alloc_block(fd, INT16_CHAIN_SZ, tmp) == NULL) return EOF;
        memcpy(index+16, tmp, 8); // 记录第一个链表的指针
        sync_block(fd, index); // 同步索引到文件
        memset(tmp+10, 0, INT16_CHAIN_SZ); // 清空
        memcpy(tmp+10+8, buf, 8); // 写入 item 位置
        return sync_block(fd, tmp+10); // 同步链表到文件
    }

    if(isexist) { // 索引已存在，仅替换指针
        ptr = le64(index+16);
        for(int i = 0; i < sum/256; i++) {
            if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
            readle64(fd, ptr);
        }
        #ifdef DEBUG
            printf("seek: %016llx, ", ptr);
        #endif
        if(lseek(fd, ptr+8*(sum%256+1), SEEK_SET) < 0) return EOF;
        #ifdef DEBUG
            puts("replace");
        #endif
        return write(fd, buf, 8) != 8;
    }

    // 索引不存在，需要搬移，统一向后移一个指针
    uint64_t prev_ptr, first_ptr = 1;
    ptr = le64(index+16);
    if(total%256 == 255) { // 旧链表刚好装满，需要新分配一个
        while(ptr && first_ptr) { // 遍历到末尾
            prev_ptr = ptr;
            if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
            readle64(fd, ptr);
            readle64(fd, first_ptr);
            if(unlikely(ptr == prev_ptr)) { // 文件损坏
                errno = ESPIPE;
                return EOF;
            }
        }
        if(first_ptr) { // 需要分配
            if(alloc_block(fd, INT16_CHAIN_SZ, tmp) == NULL) return EOF;
            lseek(fd, prev_ptr, SEEK_SET);
            write(fd, tmp, 8); // 将新分配的块附加到链表
            prev_ptr = ptr;
            ptr = le64(tmp);
            memset(tmp+10, 0, INT16_CHAIN_SZ); // 清空新链表
            sync_block(fd, tmp+10);
        }
        // 存在之前分配好的，但是由于删除索引而弃用的块，因此无需新分配
    }
    if(sum == total) { // 恰好在最后添加
        while(ptr && first_ptr) { // 遍历到末尾
            prev_ptr = ptr;
            if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
            readle64(fd, ptr);
            readle64(fd, first_ptr);
            if(unlikely(ptr == prev_ptr)) { // 文件损坏
                errno = ESPIPE;
                return EOF;
            }
        }
        #ifdef DEBUG
            printf("set: %016llx, ", prev_ptr+8*((sum+1)%256+1));
        #endif
        // 定位到最后一个未满块或第一个空块上的最后
        lseek(fd, prev_ptr+8*((sum+1)%256+1), SEEK_SET);
        #ifdef DEBUG
            puts("append");
        #endif
        return write(fd, buf, 8) != 8; // 写入
    }
    // 定位回链表头
    lseek(fd, le64(index+16), SEEK_SET);
    // 跳转到应当存入的块
    for(int i = 0; i < sum/256; i++) {
        readle64(fd, ptr);
        if(unlikely(!ptr)) {
            errno = ESPIPE;
            return EOF; // 不应当出现，如果出现说明文件损坏
        }
        lseek(fd, ptr, SEEK_SET);
    }
    // 搬移
    int offset = sum%256; // 搬移开始的位置，也是应当存入的位置
    readle64(fd, ptr); // 下一个块指针
    #ifdef DEBUG
        printf("off: %d, ", offset);
    #endif
    if(offset) { // 具有偏移，先定位到偏移
        lseek(fd, offset*8, SEEK_CUR);
    }
    readle64(fd, prev_ptr); // 读取第一个 item 指针
    #ifdef DEBUG
        printf("first item: %llu\n", prev_ptr);
    #endif
    lseek(fd, -8, SEEK_CUR); // 返回
    write(fd, buf, 8); // 插入
    while(prev_ptr) { // 一直搬移到末尾
        if(!(++offset%256)) { // 进入新的块
            lseek(fd, ptr, SEEK_SET);
            readle64(fd, first_ptr); // 下一个块指针
            if(unlikely(first_ptr == ptr)) { // 文件损坏
                errno = ESPIPE;
                return EOF;
            }
            ptr = first_ptr;
        }
        putle64(buf, prev_ptr); // 以备写入
        readle64(fd, prev_ptr); // 读取下一个 item 指针
        lseek(fd, -8, SEEK_CUR); // 返回
        write(fd, buf, 8); // 搬移一个指针
    }
    return 0;
}

uint64_t find_item_by_int16_key(int fd, void* index, key_t k) {
    uint64_t ptr;
    int isexist, sum = 0;
    uint16_t key = (uint16_t)k;
    if(key < 32768) {
        // key是否已存在
        isexist = ((uint8_t*)(index+INT16_INDEX_SZ+8))[key/8] & (128>>(key%8));
        if(!isexist) return 0;
        // 查找 key 之前共有多少索引
        for(int i = key/256*32; i < key/8; i++) { // 从未计算的32位组开始算起
            sum += __builtin_popcount(((uint8_t*)(index+INT16_INDEX_SZ+8))[i]);
        }
        sum += __builtin_popcount(((uint8_t*)(index+INT16_INDEX_SZ+8))[key/8] & ~(0xff>>(key%8)));
        #ifdef DEBUG
            printf("popc: %d, ", sum);
        #endif
        for(int i = 0; i < key/256; i++) {
            int s = ((uint8_t*)(index+24))[i];
            if(!s && ((uint8_t*)(index+INT16_INDEX_SZ+8))[i*32]) {
                sum += 256;
                continue;
            }
            sum += s;
        }
        #ifdef DEBUG
            printf("sum: %d, ", sum);
        #endif
    } else {
        // key是否已存在
        isexist = ((uint8_t*)(index+INT16_INDEX_SZ+8+8))[key/8] & (128>>(key%8));
        if(!isexist) return 0;
        // 查找 key 之前共有多少索引
        for(int i = key/256*32; i < key/8; i++) { // 从未计算的32位组开始算起
            sum += __builtin_popcount(((uint8_t*)(index+INT16_INDEX_SZ+8+8))[i]);
        }
        sum += __builtin_popcount(((uint8_t*)(index+INT16_INDEX_SZ+8+8))[key/8] & ~(0xff>>(key%8)));
        #ifdef DEBUG
            printf("popc: %d, ", sum);
        #endif
        for(int i = 0; i < 128; i++) {
            int s = ((uint8_t*)(index+24))[i];
            if(!s && ((uint8_t*)(index+INT16_INDEX_SZ+8))[i*32]) {
                sum += 256;
                continue;
            }
            sum += s;
        }
        for(int i = 128; i < key/256; i++) {
            int s = ((uint8_t*)(index+24))[i];
            if(!s && ((uint8_t*)(index+INT16_INDEX_SZ+8+8))[i*32]) {
                sum += 256;
                continue;
            }
            sum += s;
        }
        #ifdef DEBUG
            printf("sum: %d, ", sum);
        #endif
    }
    ptr = le64(index+16);
    for(int i = 0; i < sum/256; i++) {
        if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
        readle64(fd, ptr);
    }
    #ifdef DEBUG
        printf("seek: %016llx, ", ptr);
    #endif
    if(lseek(fd, ptr+8*(sum%256+1), SEEK_SET) < 0) return EOF;
    readle64(fd, ptr);
    #ifdef DEBUG
        printf("ptr: %d\n", (int)ptr);
    #endif
    return ptr;
}

#define clear_bitmap(fd, index, key) {\
    /* 写入位图 */\
    if(key<32768) {\
        ((uint8_t*)(index+INT16_INDEX_SZ+8))[key/8] &= ~(128>>(key%8));\
        if(unlikely(sync_page(fd, index+INT16_INDEX_SZ+8))) { /* 失败，撤销更改 */\
            ((uint8_t*)(index+INT16_INDEX_SZ+8))[key/8] |= 128>>(key%8);\
            return EOF;\
        }\
    } else {\
        ((uint8_t*)(index+INT16_INDEX_SZ+8+8))[key/8] &= ~(128>>(key%8));\
        if(unlikely(sync_page(fd, index+INT16_INDEX_SZ+8+PAGESZ+8))) { /* 失败，撤销更改 */\
            ((uint8_t*)(index+INT16_INDEX_SZ+8+8))[key/8] |= 128>>(key%8);\
            return EOF;\
        }\
    }\
    ((uint8_t*)(index+24))[key/256]--; /* 写入位图索引 */\
    if(unlikely(sync_block(fd, index))) { /* 失败，撤销更改 */\
        ((uint8_t*)(index+24))[key/256]++;\
        return EOF;\
    }\
}

uint64_t remove_item_by_int16_key(int fd, void* index, key_t k) {
    uint64_t ptr;
    int isexist, sum = 0;
    uint16_t key = (uint16_t)k;
    char buf[8];

    #ifdef DEBUG
        printf("No.%u: ", (int)key);
    #endif

    if(key < 32768) {
        // key是否已存在
        isexist = ((uint8_t*)(index+INT16_INDEX_SZ+8))[key/8] & (128>>(key%8));
        if(!isexist) return 0;
        // 查找 key 之前共有多少索引
        for(int i = key/256*32; i < key/8; i++) { // 从未计算的32位组开始算起
            sum += __builtin_popcount(((uint8_t*)(index+INT16_INDEX_SZ+8))[i]);
        }
        sum += __builtin_popcount(((uint8_t*)(index+INT16_INDEX_SZ+8))[key/8] & ~(0xff>>(key%8)));
        #ifdef DEBUG
            printf("popc: %d, ", sum);
        #endif
        for(int i = 0; i < key/256; i++) {
            int s = ((uint8_t*)(index+24))[i];
            if(!s && ((uint8_t*)(index+INT16_INDEX_SZ+8))[i*32]) {
                sum += 256;
                continue;
            }
            sum += s;
        }
        #ifdef DEBUG
            printf("sum: %d, ", sum);
        #endif
    } else {
        // key是否已存在
        isexist = ((uint8_t*)(index+INT16_INDEX_SZ+8+8))[key/8] & (128>>(key%8));
        if(!isexist) return 0;
        // 查找 key 之前共有多少索引
        for(int i = key/256*32; i < key/8; i++) { // 从未计算的32位组开始算起
            sum += __builtin_popcount(((uint8_t*)(index+INT16_INDEX_SZ+8+8))[i]);
        }
        sum += __builtin_popcount(((uint8_t*)(index+INT16_INDEX_SZ+8+8))[key/8] & ~(0xff>>(key%8)));
        #ifdef DEBUG
            printf("popc: %d, ", sum);
        #endif
        for(int i = 0; i < 128; i++) {
            int s = ((uint8_t*)(index+24))[i];
            if(!s && ((uint8_t*)(index+INT16_INDEX_SZ+8))[i*32]) {
                sum += 256;
                continue;
            }
            sum += s;
        }
        for(int i = 128; i < key/256; i++) {
            int s = ((uint8_t*)(index+24))[i];
            if(!s && ((uint8_t*)(index+INT16_INDEX_SZ+8+8))[i*32]) {
                sum += 256;
                continue;
            }
            sum += s;
        }
        #ifdef DEBUG
            printf("sum: %d, ", sum);
        #endif
    }

    clear_bitmap(fd, index, key);
    #ifdef DEBUG
        printf("i: %d, sumblk: %d, map: %02x, ", key/256, ((uint8_t*)(index+24))[key/256], ((uint8_t*)(index+INT16_INDEX_SZ+8+((key<32768)?0:8)))[key/8]);
    #endif

    uint64_t cur_ptr, next_ptr, first_ptr = 0;
    int offset = sum%256;

    ptr = le64(index+16);
    for(int i = 0; i < sum/256; i++) {
        if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
        readle64(fd, ptr);
    }
    if(lseek(fd, ptr, SEEK_SET) < 0) return EOF;
    readle64(fd, next_ptr);
    if(lseek(fd, 8*offset, SEEK_CUR) < 0) return EOF;
    readle64(fd, ptr);
    #ifdef DEBUG
        printf("ptr: %d, ", (int)ptr);
    #endif
    cur_ptr = lseek(fd, 0, SEEK_CUR); // 回到开头
    #ifdef DEBUG
        printf("cur: %016llx, next: %016llx\n", cur_ptr, next_ptr);
    #endif

    do { // 一直循环到末尾
        if(!(++offset%256)) { // 当前位于末尾，需要从下一页取值
            // 换下一页
            if(lseek(fd, next_ptr+8, SEEK_SET) < 0) return EOF;
            readle64(fd, first_ptr); // 读后一个值
            // 回上一页
            lseek(fd, cur_ptr, SEEK_SET);
            putle64(buf, first_ptr);
            write(fd, buf, 8); // 覆盖
            // 换下一页
            cur_ptr = lseek(fd, next_ptr, SEEK_SET);
            readle64(fd, next_ptr);
            lseek(fd, 8, SEEK_CUR);
            continue;
        }
        // 当前后方至少还有一个指针，可以直接取值
        readle64(fd, first_ptr); // 读下一个值
        lseek(fd, -16, SEEK_CUR); // 回原处
        putle64(buf, first_ptr);
        write(fd, buf, 8); // 覆盖
        cur_ptr = lseek(fd, 8, SEEK_CUR)-8;
    } while(first_ptr);

    return ptr;
}
