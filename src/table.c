#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "../include/binary.h"
#include "../include/file.h"
#include "../include/page.h"
#include "../include/table.h"
#include "../include/types.h"
#include "../include/types/int8.h"
#include "../include/types/int16.h"

// 计算索引需要的内存
static int _calc_index_size(type_t t) {
    switch(t&7) {
        case TYPE_INT8:
            return INT8_INDEX_SZ+10;
        case TYPE_INT16:
            return INT16_INDEX_SZ+10 + INT16_BITMAP_SZ+8*2;
        case TYPE_INT32:
        case TYPE_FLOAT:
        case TYPE_INT64:
        case TYPE_DOUBLE:
        case TYPE_STRING:
            return PAGESZ+8;
        case TYPE_BINARY: // 不能创建索引
        default:
            return EOF;
    }
}

// 为 t 类型创建索引，写入 index_ptr
// 返回：
//    1  失败，详见 errno
//    0  成功
static int _add_index_type(int fd, uint64_t* index_ptr, type_t t) {
    int sz = _calc_index_size(t);
    if(sz <= 0) {
        errno = EINVAL;
        return EINVAL;
    }
    void* buf = malloc(sz);
    if(buf == NULL) return 1;
    void* index = create_index(fd, t, buf);
    if(index == NULL) {
        free(buf);
        return 2;
    }
    *index_ptr = le64(buf);
    free(buf);
    return 0;
}

// 移除 t 类型索引
// 返回：
//    1  失败，详见 errno
//    0  成功
static int _remove_index_type(int fd, type_t t, uint64_t ptr) {
    int sz = _calc_index_size(t);
    if(sz <= 0) {
        errno = EINVAL;
        return EINVAL;
    }
    void* buf = malloc(sz);
    if(buf == NULL) return 1;
    void* index = load_index(fd, t, ptr, buf);
    if(index == NULL) {
        free(buf);
        return 2;
    }
    sz = remove_index(fd, t, index);
    free(buf);
    return sz;
}

// 创建表，可变参数为本表的一行的 types，详见 types.h
// 如果 types 为外键，需要紧跟一个 uint64_t ptr
// 指示外键链接到的表位置
// len(buf) >= 4096+8+2=4106
// 返回：
//    NULL  失败，详见 errno
//    table 指向表头的指针
void* create_table(int fd, char* buf, const char* name, int row_len, const void* list) {
    if(row_len <= 0 || row_len > 128) {
        errno = EINVAL;
        return NULL;
    }

    int len = strlen(name);
    if(len <= 0 || len >= UINT16_MAX) {
        errno = EINVAL;
        return NULL;
    }

    void* table = alloc_page(fd, buf+2);
    if(table == NULL) return NULL;

    memset(table, 0, PAGESZ);
    putle16(table+8, len);
    memcpy(table+8+2, name, len);
    len += 8+2;

    uint64_t ptr = get_first_table(fd);
    while(ptr > 0) ptr = get_next_table(fd, ptr); // 结束时，fd位于最后一个表头+8
    if(ptr) return NULL; // 发生错误
    lseek(fd, -8, SEEK_CUR);
    if(write(fd, buf+2, 8) != 8) return NULL; // 写入新表位置

    putle16(table+len, row_len);
    len += 2;

    int foreign_cnt = 0, ap = 0;

    type_t t = ((type_t*)list)[ap++]; // 是主键，检查是否有 unique + nonnull 类型修饰符
    if(!(t&EXTYPE_NONNULL) || !(t&EXTYPE_UNIQUE)) {
        errno = EINVAL;
        return NULL;
    }
    ((type_t*)table)[len] = t; // 填充 type of row No.0
    #ifdef DEBUG
        printf("fill[%d]: %d\n", ap, t);
    #endif
    if(t & EXTYPE_FOREIGNKEY) { // 是外键，还有一个参数
        ptr = ((uint64_t*)list)[ap];
        #ifdef DEBUG
            printf("fill[%d]: %016llx\n", ap, ptr);
        #endif
        ap += 8;
        putle64(table+len+(int)row_len*(8+1)+8*(foreign_cnt++), ptr);
    }

    // 为 pk 创建索引
    if(_add_index_type(fd, table+len+(int)row_len, t)) return NULL;
    for(int i = 1; i < (int)row_len; i++) {
        t = ((type_t*)list)[ap++];
        ((type_t*)table)[len+i] = t; // 填充 type of row No.i
        #ifdef DEBUG
            printf("fill[%d]: %d\n", ap, t);
        #endif
        if(t & EXTYPE_FOREIGNKEY) { // 是外键，还有一个参数
            ptr = ((uint64_t*)list)[ap];
            #ifdef DEBUG
                printf("fill[%d]: %016llx\n", ap, ptr);
            #endif
            ap += 8;
            putle64(table+len+(int)row_len*(8+1)+8*(foreign_cnt++), ptr);
        }
    }

    len += (int)row_len*(8+1) + 8*foreign_cnt;
    if(len > PAGESZ) {
        errno = EFBIG;
        return NULL;
    }

    // 写回文件
    if(sync_page(fd, table)) return NULL;
    // 回收剩余空间
    ptr = le64(buf+2);
    if(PAGESZ - len > 10) add_block(fd, PAGESZ - len, ptr+(uint64_t)len);
    // 将 page 变为 block
    putle64(buf, ptr);
    putle16(buf+8, len);
    #ifdef DEBUG
        printf("create len: %d\n", len);
    #endif
    return buf+10;
}

// 加载 ptr 位置的表
// len(buf) >= 4096+8+2=4106
// 返回：
//    NULL  失败，详见 errno
//    table 指向表头的指针
void* load_table(int fd, char* buf, uint64_t ptr) {
    uint64_t tmp;
    int len = 8;
    putle64(buf, ptr); // this blk ptr
    if(lseek(fd, ptr+8, SEEK_SET) < 0) return NULL; // skip next table ptr
    readle16(fd, tmp); // table name length
    len += 2+(int)tmp;
    if(lseek(fd, tmp, SEEK_CUR) < 0) return NULL;
    readle16(fd, tmp); // table row length
    len += 2+((int)tmp)*(8+1);
    for(int i = 0; i < (int)tmp; i++) {
        type_t t;
        read(fd, &t, 1);
        if(t&EXTYPE_FOREIGNKEY) { // 外键有额外 ptr 长度
            len += 8;
        }
    }
    putle16(buf+8, len); // this blk len
    lseek(fd, ptr, SEEK_SET);
    if(read(fd, buf+10, len) != len) return NULL;
    #ifdef DEBUG
        printf("load len: %d\n", len);
    #endif
    return buf+10;
}

// 获得表名长度，包含结尾0
uint16_t get_table_name_length(void* table) {
    return le16(table+8)+1;
}

// 获得表名，写入buf，len(buf) >= len(name)
// 返回：buf
char* get_table_name(void* table, char* buf) {
    uint16_t len = le16(table+8);
    memcpy(buf, table+10, len);
    buf[len] = 0;
    return buf;
}

uint64_t get_index_ptr(void* table, uint16_t pos) {
    int len = 8+2+le16(table+8);
    uint16_t rlen = le16(table+len);
    if(pos >= rlen) {
        errno = EINVAL;
        return 0;
    }
    len += 2;
    type_t t = ((type_t*)table)[len+(int)pos];
    len += (int)rlen+(int)pos*8;
    return *(uint64_t*)(table+len);
}

// 为 pos 位置的列创建索引。不可用于 0 列，即 pk 列，因为 pk 必有索引
// 返回：
//    0  失败，详见 errno
//    ptr 指向索引头的指针
uint64_t add_table_index(int fd, void* table, uint16_t pos) {
    if(!pos) {
        errno = EINVAL;
        return 0;
    }
    int len = 8+2+le16(table+8);
    uint16_t rlen = le16(table+len);
    if(pos >= rlen) {
        errno = EINVAL;
        return 0;
    }
    len += 2;
    type_t t = ((type_t*)table)[len+(int)pos];
    len += (int)rlen+(int)pos*8;
    if(*(uint64_t*)(table+len)) return *(uint64_t*)(table+len); // 已经有索引
    if(_add_index_type(fd, table+len, t)) return 0;
    if(sync_block(fd, table)) return 0;
    return *(uint64_t*)(table+len);
}

// 删除 pos 位置的列的索引。不可用于 0 列，即 pk 列，因为 pk 必有索引
// 返回：
//    非 0  失败，详见 errno
//    0     成功
int remove_table_index(int fd, void* table, uint16_t pos) {
    if(!pos) {
        errno = EINVAL;
        return 0;
    }
    int len = 8+2+le16(table+8);
    uint16_t rlen = le16(table+len);
    if(pos >= rlen) {
        errno = EINVAL;
        return 0;
    }
    len += 2;
    type_t t = ((type_t*)table)[len+(int)pos];
    len += (int)rlen+(int)pos*8;
    uint64_t ptr = *(uint64_t*)(table+len);
    if(ptr == 0) return 0; // 没有索引
    *(uint64_t*)(table+len) = 0; // 清除
    _remove_index_type(fd, t, ptr);
    return sync_block(fd, table);
}

// 插入一行，如果 pk 有值则替换
// 如果当前项有 nullable 属性，需要在此项之前
// 加一个 int isavailable，标记本项是否有值
// 如果 isavailable==0，后面不再跟有本项数据
// 如果 isavailable!=0，则在后面附加数据
// 返回：
//    0   失败，详见 errno
//    ptr 本行插入的位置
uint64_t insert_row(int fd, void* table, int row_len, const void* list) {
    return 0;
}

// 根据主键的匹配值查找行
// 如果主键不为 string，k 直接装填其值
// 否则，k 是指向 string 的指针 (const char*)
// 返回：
//    0  失败，详见 errno
//    ptr  行所在位置
uint64_t find_row_by_pk(int fd, void* table, key_t k) {
    return 0;
}

// 根据任意匹配值遍历查找行
// 可变参数两两成对，uint16_t pos + key_t val
// 如果 val 不为 string，直接装填其值
// 否则，值是指向 string 的指针 (const char*)
// f 为遍历函数，入参为本行 ptr，返回非 0 值中断遍历
// 返回：
//    非 0  失败，详见 errno
//    0     成功
int find_row_by(int fd, void* table, int (*f)(uint64_t), int row_len, const void* list) {
    return 1;
}

// 根据主键的匹配值删除行
// 如果主键不为 string，k 直接装填其值
// 否则，k 是指向 string 的指针 (const char*)
// 返回：
//    非 0  失败，详见 errno
//    0     成功
int remove_row_by_pk(int fd, void* table, key_t k) {
    return 1;
}

// 根据任意匹配值删除行
// 可变参数两两成对，uint16_t pos + key_t val
// 如果 val 不为 string，直接装填其值
// 否则，值是指向 string 的指针 (const char*)
// 返回：
//    非 0  失败，详见 errno
//    0     成功
int remove_row_by(int fd, void* table, int row_len, const void* list) {
    return 1;
}
