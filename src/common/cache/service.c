#include "service.h"

#include "core.h"

#include <string.h>

int cache_service_init(cache_service_t *service, size_t capacity_pages, size_t page_size) {
    if (!service) {
        return -1;
    }

    if (service->ready) {
        return 0;
    }

    if (page_cache_init(&service->cache, capacity_pages, page_size) != 0) {
        return -1;
    }

    service->capacity_pages = capacity_pages;
    service->page_size = page_size;
    service->ready = 1;
    return 0;
}

void cache_service_shutdown(cache_service_t *service) {
    if (!service || !service->ready) {
        return;
    }

    page_cache_destroy(&service->cache);
    memset(service, 0, sizeof(*service));
}

int cache_service_clear(cache_service_t *service) {
    size_t capacity_pages;
    size_t page_size;

    if (!service || !service->ready) {
        return -1;
    }

    capacity_pages = service->capacity_pages;
    page_size = service->page_size;
    cache_service_shutdown(service);
    return cache_service_init(service, capacity_pages, page_size);
}

int cache_service_get(cache_service_t *service, uint32_t page_id, uint8_t *out_buffer, size_t *inout_len) {
    page_cache_entry_t *entry;

    if (!service || !service->ready || !out_buffer || !inout_len) {
        return -1;
    }

    entry = page_cache_get(&service->cache, page_id);
    if (!entry || *inout_len < entry->data_len) {
        return -1;
    }

    memcpy(out_buffer, entry->data, entry->data_len);
    *inout_len = entry->data_len;
    return 0;
}

int cache_service_get_ptr(cache_service_t *service, uint32_t page_id, const uint8_t **out_data, size_t *out_len) {
    page_cache_entry_t *entry;

    if (!service || !service->ready || !out_data || !out_len) {
        return -1;
    }

    entry = page_cache_get(&service->cache, page_id);
    if (!entry) {
        return -1;
    }

    *out_data = entry->data;
    *out_len = entry->data_len;
    return 0;
}

int cache_service_set(cache_service_t *service, uint32_t page_id, const uint8_t *data, size_t data_len) {
    if (!service || !service->ready) {
        return -1;
    }

    return page_cache_put(&service->cache, page_id, data, data_len);
}

int cache_service_has(cache_service_t *service, uint32_t page_id) {
    if (!service || !service->ready) {
        return 0;
    }

    return page_cache_has(&service->cache, page_id);
}

int cache_service_prefetch(cache_service_t *service, const uint32_t *page_ids, size_t count) {
    size_t i;

    if (!service || !service->ready || (!page_ids && count > 0)) {
        return -1;
    }

    for (i = 0; i < count; ++i) {
        if (page_cache_touch(&service->cache, page_ids[i]) == NULL) {
            return -1;
        }
    }

    return 0;
}

int cache_service_pin(cache_service_t *service, uint32_t page_id) {
    if (!service || !service->ready) {
        return -1;
    }

    return page_cache_pin(&service->cache, page_id);
}

int cache_service_release(cache_service_t *service, uint32_t page_id) {
    if (!service || !service->ready) {
        return -1;
    }

    return page_cache_unpin(&service->cache, page_id);
}

int cache_service_stats(cache_service_t *service, cache_service_stats_t *out_stats) {
    if (!service || !service->ready || !out_stats) {
        return -1;
    }

    out_stats->hits = service->cache.hits;
    out_stats->misses = service->cache.misses;
    out_stats->evictions = service->cache.evictions;
    out_stats->entries = service->cache.count;
    out_stats->reserved_bytes = service->cache.total_reserved_bytes;
    out_stats->metadata_bytes = service->cache.metadata_bytes;
    out_stats->hash_index_bytes = service->cache.hash_index_bytes;
    out_stats->payload_capacity_bytes = service->cache.payload_capacity_bytes;
    return 0;
}
