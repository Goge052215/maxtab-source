#ifndef CACHE_SERVICE_NATIVE_H
#define CACHE_SERVICE_NATIVE_H

#include <stddef.h>
#include <stdint.h>

#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t hits;
    uint64_t misses;
    uint64_t evictions;
    size_t entries;
    size_t reserved_bytes;
    size_t metadata_bytes;
    size_t hash_index_bytes;
    size_t payload_capacity_bytes;
} cache_service_stats_t;

typedef struct {
    page_cache_t cache;
    size_t capacity_pages;
    size_t page_size;
    int ready;
} cache_service_t;

int cache_service_init(cache_service_t *service, size_t capacity_pages, size_t page_size);
void cache_service_shutdown(cache_service_t *service);
int cache_service_clear(cache_service_t *service);
int cache_service_get(cache_service_t *service, uint32_t page_id, uint8_t *out_buffer, size_t *inout_len);
int cache_service_get_ptr(cache_service_t *service, uint32_t page_id, const uint8_t **out_data, size_t *out_len);
int cache_service_set(cache_service_t *service, uint32_t page_id, const uint8_t *data, size_t data_len);
int cache_service_has(cache_service_t *service, uint32_t page_id);
int cache_service_prefetch(cache_service_t *service, const uint32_t *page_ids, size_t count);
int cache_service_pin(cache_service_t *service, uint32_t page_id);
int cache_service_release(cache_service_t *service, uint32_t page_id);
int cache_service_stats(cache_service_t *service, cache_service_stats_t *out_stats);

#ifdef __cplusplus
}
#endif

#endif
