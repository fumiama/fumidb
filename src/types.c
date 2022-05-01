#include "../include/types.h"
#include "../include/types/int8.h"

// ptr = init(fd)
typedef void* (*_type_init_t)(int);
// ptr = load(fd, ptr)
typedef void* (*_type_load_t)(int, uint64_t);
// ret = insert_item(fd, index, k, ptr)
typedef int (*_insert_item_t)(int, void*, key_t, uint64_t);
// ptr = find_by_key(fd, index, k)
typedef uint64_t (*_find_by_key_t)(int, void*, key_t);
// ret = remove_by_key(fd, index, k)
typedef int (*_remove_by_key_t)(int, void*, key_t);

// Function not implemented
static void* create_not_impl_index(int fd) {
    errno = ENOSYS;
    return 0;
}

// Function not implemented
static void* load_not_impl_index(int fd, uint64_t ptr) {
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
static int remove_item_by_not_impl_key(int fd, void* index, key_t k) {
    errno = ENOSYS;
    return 0;
}

static _type_init_t _types_init[] = {
    create_int8_index,
    create_not_impl_index,
    create_not_impl_index,
    create_not_impl_index,
    create_not_impl_index,
    create_not_impl_index,
    create_not_impl_index
};

static _type_load_t _types_load[] = {
    load_int8_index,
    load_not_impl_index,
    load_not_impl_index,
    load_not_impl_index,
    load_not_impl_index,
    load_not_impl_index,
    load_not_impl_index
};

static _insert_item_t _insert_item[] = {
    insert_int8_item,
    insert_not_impl_item,
    insert_not_impl_item,
    insert_not_impl_item,
    insert_not_impl_item,
    insert_not_impl_item,
    insert_not_impl_item
};

static _find_by_key_t _find_item_by_key[] = {
    find_item_by_int8_key,
    find_item_by_not_impl_key,
    find_item_by_not_impl_key,
    find_item_by_not_impl_key,
    find_item_by_not_impl_key,
    find_item_by_not_impl_key,
    find_item_by_not_impl_key
};

static _remove_by_key_t _remove_item_by_key[] = {
    remove_item_by_int8_key,
    remove_item_by_not_impl_key,
    remove_item_by_not_impl_key,
    remove_item_by_not_impl_key,
    remove_item_by_not_impl_key,
    remove_item_by_not_impl_key,
    remove_item_by_not_impl_key
};

void* create_index(int fd, type_t t) {
    return _types_init[t&7](fd);
}

void* load_index(int fd, type_t t, uint64_t ptr) {
    return _types_load[t&7](fd, ptr);
}

int insert_item(int fd, type_t t, void* index, key_t k, uint64_t ptr) {
    return _insert_item[t&7](fd, index, k, ptr);
}

uint64_t find_item_by_key(int fd, type_t t, void* index, key_t k) {
    return _find_item_by_key[t&7](fd, index, k);
}

int remove_item_by_key(int fd, type_t t, void* index, key_t k) {
    return _remove_item_by_key[t&7](fd, index, k);
}
