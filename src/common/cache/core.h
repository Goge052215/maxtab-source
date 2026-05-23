#ifndef CACHE_CORE_H
#define CACHE_CORE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t page_id;
    uint32_t data_len;
    uint16_t pin_count;
    uint8_t occupied;
    uint8_t referenced;
    uint8_t *data;
} page_cache_entry_t;

typedef struct {
    size_t capacity_pages;
    size_t page_size;
    size_t count;
    size_t clock_hand;
    size_t hash_capacity;
    size_t payload_capacity_bytes;
    size_t metadata_bytes;
    size_t hash_index_bytes;
    size_t total_reserved_bytes;
    size_t peak_reserved_bytes;
    uint64_t hits;
    uint64_t misses;
    uint64_t evictions;
    page_cache_entry_t *entries;
    uint8_t *storage;
    uint32_t *hash_keys;
    size_t *hash_slots;
    uint8_t *hash_states;
} page_cache_t;

int page_cache_init(page_cache_t *cache, size_t capacity_pages, size_t page_size);
void page_cache_destroy(page_cache_t *cache);
page_cache_entry_t *page_cache_get(page_cache_t *cache, uint32_t page_id);
page_cache_entry_t *page_cache_touch(page_cache_t *cache, uint32_t page_id);
int page_cache_put(page_cache_t *cache, uint32_t page_id, const uint8_t *data, size_t data_len);
int page_cache_has(page_cache_t *cache, uint32_t page_id);
int page_cache_pin(page_cache_t *cache, uint32_t page_id);
int page_cache_unpin(page_cache_t *cache, uint32_t page_id);
size_t page_cache_reserved_bytes(const page_cache_t *cache);
int page_cache_smoke_test(void);

#ifdef __cplusplus
}
#endif

#endif
