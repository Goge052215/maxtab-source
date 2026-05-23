#include "core.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

enum {
    PAGE_CACHE_HASH_EMPTY = 0,
    PAGE_CACHE_HASH_OCCUPIED = 1,
    PAGE_CACHE_HASH_TOMBSTONE = 2
};

static size_t page_cache_next_power_of_two(size_t value) {
    size_t result = 1;
    while (result < value) {
        result <<= 1;
    }
    return result;
}

static size_t page_cache_hash_key(uint32_t page_id) {
    uint32_t mixed = page_id * 2654435761u;
    mixed ^= mixed >> 16;
    return (size_t)mixed;
}

static size_t page_cache_slot_of(page_cache_t *cache, page_cache_entry_t *entry) {
    return (size_t)(entry - cache->entries);
}

static int page_cache_mul_overflows(size_t left, size_t right) {
    return right != 0 && left > (SIZE_MAX / right);
}

static size_t page_cache_hash_probe(page_cache_t *cache, uint32_t page_id, int *found) {
    size_t index = page_cache_hash_key(page_id) & (cache->hash_capacity - 1);
    size_t first_tombstone = SIZE_MAX;

    for (;;) {
        uint8_t state = cache->hash_states[index];
        if (state == PAGE_CACHE_HASH_EMPTY) {
            *found = 0;
            return first_tombstone != SIZE_MAX ? first_tombstone : index;
        }

        if (state == PAGE_CACHE_HASH_OCCUPIED && cache->hash_keys[index] == page_id) {
            *found = 1;
            return index;
        }

        if (state == PAGE_CACHE_HASH_TOMBSTONE && first_tombstone == SIZE_MAX) {
            first_tombstone = index;
        }

        index = (index + 1) & (cache->hash_capacity - 1);
    }
}

static page_cache_entry_t *page_cache_find(page_cache_t *cache, uint32_t page_id) {
    int found = 0;
    size_t index;

    if (!cache || cache->hash_capacity == 0) {
        return NULL;
    }

    index = page_cache_hash_probe(cache, page_id, &found);
    if (!found) {
        return NULL;
    }

    return &cache->entries[cache->hash_slots[index]];
}

static void page_cache_hash_insert(page_cache_t *cache, uint32_t page_id, size_t slot) {
    int found = 0;
    size_t index = page_cache_hash_probe(cache, page_id, &found);
    cache->hash_keys[index] = page_id;
    cache->hash_slots[index] = slot;
    cache->hash_states[index] = PAGE_CACHE_HASH_OCCUPIED;
}

static void page_cache_hash_remove(page_cache_t *cache, uint32_t page_id) {
    int found = 0;
    size_t index = page_cache_hash_probe(cache, page_id, &found);
    if (!found) {
        return;
    }

    cache->hash_states[index] = PAGE_CACHE_HASH_TOMBSTONE;
}

static void page_cache_fill(page_cache_entry_t *entry, uint32_t page_id, size_t page_size) {
    size_t i;
    entry->page_id = page_id;
    entry->data_len = (uint32_t)page_size;
    entry->pin_count = 0;
    entry->occupied = 1;
    entry->referenced = 1;

    for (i = 0; i < page_size; ++i) {
        entry->data[i] = (uint8_t)((page_id + i) & 0xff);
    }
}

static void page_cache_assign(page_cache_entry_t *entry,
                              uint32_t page_id,
                              const uint8_t *data,
                              size_t data_len) {
    entry->page_id = page_id;
    entry->data_len = (uint32_t)data_len;
    entry->occupied = 1;
    entry->referenced = 1;

    if (data_len > 0 && data != NULL) {
        memcpy(entry->data, data, data_len);
    }
}

static page_cache_entry_t *page_cache_allocate_slot(page_cache_t *cache) {
    size_t scanned = 0;

    while (scanned < cache->capacity_pages * 2) {
        page_cache_entry_t *entry = &cache->entries[cache->clock_hand];

        cache->clock_hand = (cache->clock_hand + 1) % cache->capacity_pages;
        scanned += 1;

        if (!entry->occupied) {
            cache->count += 1;
            return entry;
        }

        if (entry->pin_count > 0) {
            continue;
        }

        if (entry->referenced) {
            entry->referenced = 0;
            continue;
        }

        page_cache_hash_remove(cache, entry->page_id);
        cache->evictions += 1;
        return entry;
    }

    return NULL;
}

int page_cache_init(page_cache_t *cache, size_t capacity_pages, size_t page_size) {
    size_t i;
    size_t hash_capacity;
    size_t hash_key_bytes;
    size_t hash_slot_bytes;
    size_t hash_state_bytes;
    size_t entry_bytes;
    size_t payload_bytes;

    if (!cache || capacity_pages == 0 || page_size == 0) {
        return -1;
    }

    if (page_cache_mul_overflows(capacity_pages, 4) ||
        page_cache_mul_overflows(capacity_pages, sizeof(page_cache_entry_t)) ||
        page_cache_mul_overflows(capacity_pages, page_size)) {
        return -1;
    }

    memset(cache, 0, sizeof(*cache));
    cache->capacity_pages = capacity_pages;
    cache->page_size = page_size;
    hash_capacity = page_cache_next_power_of_two(capacity_pages * 4);

    if (page_cache_mul_overflows(hash_capacity, sizeof(uint32_t)) ||
        page_cache_mul_overflows(hash_capacity, sizeof(size_t)) ||
        page_cache_mul_overflows(hash_capacity, sizeof(uint8_t))) {
        return -1;
    }

    hash_key_bytes = hash_capacity * sizeof(uint32_t);
    hash_slot_bytes = hash_capacity * sizeof(size_t);
    hash_state_bytes = hash_capacity * sizeof(uint8_t);
    entry_bytes = capacity_pages * sizeof(page_cache_entry_t);
    payload_bytes = capacity_pages * page_size;

    if (hash_key_bytes > SIZE_MAX - hash_slot_bytes ||
        hash_key_bytes + hash_slot_bytes > SIZE_MAX - hash_state_bytes ||
        entry_bytes > SIZE_MAX - (hash_key_bytes + hash_slot_bytes + hash_state_bytes) ||
        entry_bytes + hash_key_bytes + hash_slot_bytes + hash_state_bytes > SIZE_MAX - payload_bytes) {
        return -1;
    }

    cache->hash_capacity = hash_capacity;
    cache->hash_index_bytes = hash_key_bytes + hash_slot_bytes + hash_state_bytes;
    cache->metadata_bytes = entry_bytes + cache->hash_index_bytes;
    cache->payload_capacity_bytes = payload_bytes;
    cache->total_reserved_bytes = cache->metadata_bytes + cache->payload_capacity_bytes;
    cache->peak_reserved_bytes = cache->total_reserved_bytes;

    cache->entries = (page_cache_entry_t *)calloc(capacity_pages, sizeof(page_cache_entry_t));
    cache->storage = (uint8_t *)malloc(cache->payload_capacity_bytes);
    cache->hash_keys = (uint32_t *)calloc(cache->hash_capacity, sizeof(uint32_t));
    cache->hash_slots = (size_t *)calloc(cache->hash_capacity, sizeof(size_t));
    cache->hash_states = (uint8_t *)calloc(cache->hash_capacity, sizeof(uint8_t));

    if (!cache->entries || !cache->storage || !cache->hash_keys || !cache->hash_slots || !cache->hash_states) {
        page_cache_destroy(cache);
        return -1;
    }

    for (i = 0; i < capacity_pages; ++i) {
        cache->entries[i].data = cache->storage + (i * page_size);
    }

    return 0;
}

void page_cache_destroy(page_cache_t *cache) {
    if (!cache) {
        return;
    }

    free(cache->entries);
    free(cache->storage);
    free(cache->hash_keys);
    free(cache->hash_slots);
    free(cache->hash_states);
    memset(cache, 0, sizeof(*cache));
}

page_cache_entry_t *page_cache_get(page_cache_t *cache, uint32_t page_id) {
    page_cache_entry_t *entry;

    if (!cache) {
        return NULL;
    }

    entry = page_cache_find(cache, page_id);
    if (!entry) {
        cache->misses += 1;
        return NULL;
    }

    entry->referenced = 1;
    cache->hits += 1;
    return entry;
}

page_cache_entry_t *page_cache_touch(page_cache_t *cache, uint32_t page_id) {
    page_cache_entry_t *entry = page_cache_get(cache, page_id);

    if (entry) {
        return entry;
    }

    entry = page_cache_allocate_slot(cache);
    if (!entry) {
        return NULL;
    }

    page_cache_fill(entry, page_id, cache->page_size);
    page_cache_hash_insert(cache, page_id, page_cache_slot_of(cache, entry));
    return entry;
}

int page_cache_put(page_cache_t *cache, uint32_t page_id, const uint8_t *data, size_t data_len) {
    page_cache_entry_t *entry;

    if (!cache || data_len > cache->page_size || (data_len > 0 && data == NULL)) {
        return -1;
    }

    entry = page_cache_find(cache, page_id);
    if (!entry) {
        entry = page_cache_allocate_slot(cache);
        if (!entry) {
            return -1;
        }
        page_cache_hash_insert(cache, page_id, page_cache_slot_of(cache, entry));
    }

    page_cache_assign(entry, page_id, data, data_len);
    return 0;
}

int page_cache_has(page_cache_t *cache, uint32_t page_id) {
    return page_cache_find(cache, page_id) != NULL;
}

int page_cache_pin(page_cache_t *cache, uint32_t page_id) {
    page_cache_entry_t *entry = page_cache_touch(cache, page_id);
    if (!entry) {
        return -1;
    }

    entry->pin_count += 1;
    return 0;
}

int page_cache_unpin(page_cache_t *cache, uint32_t page_id) {
    page_cache_entry_t *entry = page_cache_find(cache, page_id);
    if (!entry || entry->pin_count == 0) {
        return -1;
    }

    entry->pin_count -= 1;
    return 0;
}

size_t page_cache_reserved_bytes(const page_cache_t *cache) {
    if (!cache) {
        return 0;
    }

    return cache->total_reserved_bytes;
}

int page_cache_smoke_test(void) {
    page_cache_t cache;
    page_cache_entry_t *entry;

    if (page_cache_init(&cache, 2, 32) != 0) {
        return -1;
    }

    entry = page_cache_touch(&cache, 1);
    if (!entry || entry->page_id != 1 || entry->data[0] != 1) {
        page_cache_destroy(&cache);
        return -1;
    }

    entry = page_cache_touch(&cache, 2);
    if (!entry || entry->page_id != 2) {
        page_cache_destroy(&cache);
        return -1;
    }

    if (page_cache_pin(&cache, 1) != 0) {
        page_cache_destroy(&cache);
        return -1;
    }

    entry = page_cache_touch(&cache, 3);
    if (!entry || entry->page_id != 3) {
        page_cache_destroy(&cache);
        return -1;
    }

    if (page_cache_get(&cache, 1) == NULL) {
        page_cache_destroy(&cache);
        return -1;
    }

    if (page_cache_unpin(&cache, 1) != 0) {
        page_cache_destroy(&cache);
        return -1;
    }

    if (page_cache_get(&cache, 3) == NULL) {
        page_cache_destroy(&cache);
        return -1;
    }

    page_cache_destroy(&cache);
    return 0;
}
