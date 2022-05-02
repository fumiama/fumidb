#include "../include/types.h"
#include "../include/types/int8.h"
#include "../include/types/int16.h"

// ptr = init(fd)
typedef void* (*_type_init_t)(int, void*);
// ptr = load(fd, ptr)
typedef void* (*_type_load_t)(int, uint64_t, void*);
// ret = rm(fd, index)
typedef int (*_type_remove_t)(int, void*);
// n = count(fd, index)
typedef uint64_t (*_type_count_t)(int, void*);
// ret = insert_item(fd, index, k, ptr)
typedef int (*_insert_item_t)(int, void*, key_t, uint64_t);
// ptr = find_by_key(fd, index, k)
typedef uint64_t (*_find_by_key_t)(int, void*, key_t);
// ret = remove_by_key(fd, index, k)
typedef uint64_t (*_remove_by_key_t)(int, void*, key_t);

// Function not implemented
static void* create_not_impl_index(int fd, void* buf) {
    errno = ENOSYS;
    return 0;
}

// Function not implemented
static void* load_not_impl_index(int fd, uint64_t ptr, void* buf) {
    errno = ENOSYS;
    return 0;
}

// Function not implemented
static int remove_not_impl_index(int fd, void* index) {
    errno = ENOSYS;
    return 0;
}

// Function not implemented
static uint64_t count_not_impl_items(int fd, void* index) {
    errno = ENOSYS;
    return 0;
}

// Function not implemented
static int insert_not_impl_item(int fd, void* index, key_t k, uint64_t ptr) {
    errno = ENOSYS;
    return 0;
}

// Function not implemented
static uint64_t find_item_by_not_impl_key(int fd, void* index, key_t k) {
    errno = ENOSYS;
    return 0;
}

// Function not implemented
static uint64_t remove_item_by_not_impl_key(int fd, void* index, key_t k) {
    errno = ENOSYS;
    return 0;
}

static _type_init_t _types_init[] = {
    create_int8_index,
    create_int16_index,
    create_not_impl_index,
    create_not_impl_index,
    create_not_impl_index,
    create_not_impl_index,
    create_not_impl_index
};

static _type_load_t _types_load[] = {
    load_int8_index,
    load_int16_index,
    load_not_impl_index,
    load_not_impl_index,
    load_not_impl_index,
    load_not_impl_index,
    load_not_impl_index
};

static _type_remove_t _types_remove[] = {
    remove_int8_index,
    remove_int16_index,
    remove_not_impl_index,
    remove_not_impl_index,
    remove_not_impl_index,
    remove_not_impl_index,
    remove_not_impl_index
};

static _type_count_t _types_count[] = {
    count_int8_items,
    count_int16_items,
    count_not_impl_items,
    count_not_impl_items,
    count_not_impl_items,
    count_not_impl_items,
    count_not_impl_items
};

static _insert_item_t _insert_item[] = {
    insert_int8_item,
    insert_int16_item,
    insert_not_impl_item,
    insert_not_impl_item,
    insert_not_impl_item,
    insert_not_impl_item,
    insert_not_impl_item
};

static _find_by_key_t _find_item_by_key[] = {
    find_item_by_int8_key,
    find_item_by_int16_key,
    find_item_by_not_impl_key,
    find_item_by_not_impl_key,
    find_item_by_not_impl_key,
    find_item_by_not_impl_key,
    find_item_by_not_impl_key
};

static _remove_by_key_t _remove_item_by_key[] = {
    remove_item_by_int8_key,
    remove_item_by_int16_key,
    remove_item_by_not_impl_key,
    remove_item_by_not_impl_key,
    remove_item_by_not_impl_key,
    remove_item_by_not_impl_key,
    remove_item_by_not_impl_key
};

void* create_index(int fd, type_t t, void* buf) {
    return _types_init[t&7](fd, buf);
}

void* load_index(int fd, type_t t, uint64_t ptr, void* buf) {
    return _types_load[t&7](fd, ptr, buf);
}

int remove_index(int fd, type_t t, void* index) {
    return _types_remove[t&7](fd, index);
}

uint64_t count_items(int fd, type_t t, void* index) {
    return _types_count[t&7](fd, index);
}

int insert_item(int fd, type_t t, void* index, key_t k, uint64_t ptr) {
    return _insert_item[t&7](fd, index, k, ptr);
}

uint64_t find_item_by_key(int fd, type_t t, void* index, key_t k) {
    return _find_item_by_key[t&7](fd, index, k);
}

uint64_t remove_item_by_key(int fd, type_t t, void* index, key_t k) {
    return _remove_item_by_key[t&7](fd, index, k);
}
