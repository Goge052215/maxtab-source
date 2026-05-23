#include "bridge.h"

#include "service.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Small Vela-oriented native bridge.
 *
 * It uses a small static registry and a conservative JSON parser tailored
 * to the request shapes produced by `src/common/cache/provider.js`.
 */

enum {
    CACHE_BRIDGE_MAX_CACHES = 8,
    CACHE_BRIDGE_MAX_HANDLE_LEN = 32,
    CACHE_BRIDGE_MAX_NAMESPACE_LEN = 64,
    CACHE_BRIDGE_MAX_KEY_LEN = 128,
    CACHE_BRIDGE_MAX_CAPACITY_PAGES = 1024,
    CACHE_BRIDGE_MAX_VALUE_LEN = 4096,
    CACHE_BRIDGE_MAX_RESPONSE_LEN = 8192
};

typedef struct {
    int in_use;
    char *key;
    uint32_t page_id;
} cache_bridge_key_binding_t;

typedef struct {
    int in_use;
    char handle[CACHE_BRIDGE_MAX_HANDLE_LEN];
    char cache_namespace[CACHE_BRIDGE_MAX_NAMESPACE_LEN];
    cache_service_t service;
    cache_bridge_key_binding_t *bindings;
    size_t binding_capacity;
    size_t binding_count;
    uint32_t next_page_id;
} cache_bridge_slot_t;

static cache_bridge_slot_t g_cache_slots[CACHE_BRIDGE_MAX_CACHES];
static unsigned int g_next_handle_id = 1;

static int cache_bridge_validate_config(size_t capacity_pages, size_t page_size) {
    if (capacity_pages == 0 || page_size == 0) {
        return -1;
    }

    if (capacity_pages > CACHE_BRIDGE_MAX_CAPACITY_PAGES || page_size > CACHE_BRIDGE_MAX_VALUE_LEN) {
        return -1;
    }

    return 0;
}

static void cache_bridge_release_binding(cache_bridge_key_binding_t *binding) {
    if (!binding || !binding->in_use) {
        return;
    }

    free(binding->key);
    memset(binding, 0, sizeof(*binding));
}

static void cache_bridge_release_bindings(cache_bridge_slot_t *slot) {
    size_t i;

    if (!slot || !slot->bindings) {
        return;
    }

    for (i = 0; i < slot->binding_capacity; ++i) {
        cache_bridge_release_binding(&slot->bindings[i]);
    }

    free(slot->bindings);
    slot->bindings = NULL;
    slot->binding_capacity = 0;
    slot->binding_count = 0;
    slot->next_page_id = 1;
}

static void cache_bridge_reset_slot(cache_bridge_slot_t *slot) {
    if (!slot) {
        return;
    }

    if (slot->service.ready) {
        cache_service_shutdown(&slot->service);
    }
    cache_bridge_release_bindings(slot);
    memset(slot, 0, sizeof(*slot));
}

static int cache_bridge_init_bindings(cache_bridge_slot_t *slot, size_t capacity_pages) {
    size_t binding_capacity;

    if (!slot || capacity_pages == 0) {
        return -1;
    }

    binding_capacity = capacity_pages * 2;
    if (binding_capacity < 8) {
        binding_capacity = 8;
    }

    slot->bindings =
        (cache_bridge_key_binding_t *)calloc(binding_capacity, sizeof(cache_bridge_key_binding_t));
    if (!slot->bindings) {
        return -1;
    }

    slot->binding_capacity = binding_capacity;
    slot->binding_count = 0;
    slot->next_page_id = 1;
    return 0;
}

static const char *cache_bridge_error_response(const char *message) {
    static char response[CACHE_BRIDGE_MAX_RESPONSE_LEN];
    snprintf(response, sizeof(response),
             "{\"ok\":false,\"errorMessage\":\"%s\"}",
             message ? message : "cache_bridge_error");
    return response;
}

static const char *cache_bridge_ok_response(void) {
    return "{\"ok\":true}";
}

static const char *cache_bridge_skip_whitespace(const char *cursor) {
    while (cursor && *cursor && isspace((unsigned char)*cursor)) {
        cursor += 1;
    }
    return cursor;
}

static int cache_bridge_extract_string(const char *json,
                                       const char *key,
                                       char *out_value,
                                       size_t out_size) {
    char needle[64];
    const char *cursor;
    size_t out_index = 0;

    if (!json || !key || !out_value || out_size == 0) {
        return -1;
    }

    snprintf(needle, sizeof(needle), "\"%s\"", key);
    cursor = strstr(json, needle);
    if (!cursor) {
        return -1;
    }

    cursor = strchr(cursor + strlen(needle), ':');
    if (!cursor) {
        return -1;
    }

    cursor = cache_bridge_skip_whitespace(cursor + 1);
    if (!cursor || *cursor != '"') {
        return -1;
    }

    cursor += 1;
    while (*cursor && *cursor != '"') {
        char ch = *cursor++;
        if (ch == '\\' && *cursor) {
            ch = *cursor++;
            if (ch == '"' || ch == '\\' || ch == '/') {
                /* use ch as is */
            } else if (ch == 'n') {
                ch = '\n';
            } else if (ch == 'r') {
                ch = '\r';
            } else if (ch == 't') {
                ch = '\t';
            } else if (ch == 'b') {
                ch = '\b';
            } else if (ch == 'f') {
                ch = '\f';
            } else {
                return -1;
            }
        }

        if (out_index + 1 >= out_size) {
            return -1;
        }
        out_value[out_index++] = ch;
    }

    if (*cursor != '"') {
        return -1;
    }

    out_value[out_index] = '\0';
    return 0;
}

static int cache_bridge_extract_size(const char *json, const char *key, size_t *out_value) {
    char needle[64];
    const char *cursor;
    char *end_ptr;
    unsigned long long value;

    if (!json || !key || !out_value) {
        return -1;
    }

    snprintf(needle, sizeof(needle), "\"%s\"", key);
    cursor = strstr(json, needle);
    if (!cursor) {
        return -1;
    }

    cursor = strchr(cursor + strlen(needle), ':');
    if (!cursor) {
        return -1;
    }

    cursor = cache_bridge_skip_whitespace(cursor + 1);
    if (!cursor || !isdigit((unsigned char)*cursor)) {
        return -1;
    }

    value = strtoull(cursor, &end_ptr, 10);
    if (end_ptr == cursor) {
        return -1;
    }

    *out_value = (size_t)value;
    return 0;
}

static uint32_t cache_bridge_hash_key(const char *text) {
    uint32_t hash = 2166136261u;
    unsigned char ch;

    if (!text) {
        return 0;
    }

    while (*text) {
        ch = (unsigned char)*text++;
        hash ^= (uint32_t)ch;
        hash *= 16777619u;
    }

    return hash;
}

static void cache_bridge_escape_json_string(const char *input, char *output, size_t output_size) {
    size_t out_index = 0;

    if (!output || output_size == 0) {
        return;
    }

    while (input && *input && out_index + 2 < output_size) {
        char ch = *input++;
        if (ch == '"' || ch == '\\') {
            output[out_index++] = '\\';
            output[out_index++] = ch;
        } else if (ch == '\n') {
            output[out_index++] = '\\';
            output[out_index++] = 'n';
        } else if (ch == '\r') {
            output[out_index++] = '\\';
            output[out_index++] = 'r';
        } else if (ch == '\t') {
            output[out_index++] = '\\';
            output[out_index++] = 't';
        } else {
            output[out_index++] = ch;
        }
    }

    output[out_index] = '\0';
}

static cache_bridge_slot_t *cache_bridge_find_slot(const char *handle) {
    size_t i;

    if (!handle) {
        return NULL;
    }

    for (i = 0; i < CACHE_BRIDGE_MAX_CACHES; ++i) {
        if (g_cache_slots[i].in_use && strcmp(g_cache_slots[i].handle, handle) == 0) {
            return &g_cache_slots[i];
        }
    }

    return NULL;
}

static cache_bridge_slot_t *cache_bridge_alloc_slot(void) {
    size_t i;

    for (i = 0; i < CACHE_BRIDGE_MAX_CACHES; ++i) {
        if (!g_cache_slots[i].in_use) {
            memset(&g_cache_slots[i], 0, sizeof(g_cache_slots[i]));
            g_cache_slots[i].in_use = 1;
            return &g_cache_slots[i];
        }
    }

    return NULL;
}
 
static uint32_t cache_bridge_next_page_id(cache_bridge_slot_t *slot) {
    uint32_t page_id;

    if (!slot) {
        return 0;
    }

    for (;;) {
        size_t i;
        int in_use = 0;

        page_id = slot->next_page_id++;
        if (page_id == 0) {
            page_id = slot->next_page_id++;
        }

        for (i = 0; i < slot->binding_capacity; ++i) {
            if (slot->bindings[i].in_use && slot->bindings[i].page_id == page_id) {
                in_use = 1;
                break;
            }
        }

        if (!in_use) {
            return page_id;
        }
    }
}

static void cache_bridge_compact_bindings(cache_bridge_slot_t *slot) {
    size_t i;

    if (!slot || !slot->bindings) {
        return;
    }

    for (i = 0; i < slot->binding_capacity; ++i) {
        cache_bridge_key_binding_t *binding = &slot->bindings[i];
        if (!binding->in_use) {
            continue;
        }

        if (!cache_service_has(&slot->service, binding->page_id)) {
            cache_bridge_release_binding(binding);
            if (slot->binding_count > 0) {
                slot->binding_count -= 1;
            }
        }
    }
}

static cache_bridge_key_binding_t *cache_bridge_find_binding(cache_bridge_slot_t *slot, const char *key) {
    size_t i;

    if (!slot || !slot->bindings || !key) {
        return NULL;
    }

    for (i = 0; i < slot->binding_capacity; ++i) {
        cache_bridge_key_binding_t *binding = &slot->bindings[i];
        if (!binding->in_use || !binding->key) {
            continue;
        }

        if (strcmp(binding->key, key) != 0) {
            continue;
        }

        if (!cache_service_has(&slot->service, binding->page_id)) {
            cache_bridge_release_binding(binding);
            if (slot->binding_count > 0) {
                slot->binding_count -= 1;
            }
            return NULL;
        }

        return binding;
    }

    return NULL;
}

static cache_bridge_key_binding_t *cache_bridge_bind_key(cache_bridge_slot_t *slot, const char *key) {
    size_t i;
    cache_bridge_key_binding_t *binding;
    char *key_copy;

    if (!slot || !slot->bindings || !key) {
        return NULL;
    }

    binding = cache_bridge_find_binding(slot, key);
    if (binding) {
        return binding;
    }

    if (slot->binding_count >= slot->binding_capacity) {
        cache_bridge_compact_bindings(slot);
    }

    if (slot->binding_count >= slot->binding_capacity) {
        return NULL;
    }

    key_copy = (char *)malloc(strlen(key) + 1);
    if (!key_copy) {
        return NULL;
    }
    strcpy(key_copy, key);

    for (i = 0; i < slot->binding_capacity; ++i) {
        binding = &slot->bindings[i];
        if (binding->in_use) {
            continue;
        }

        binding->in_use = 1;
        binding->key = key_copy;
        binding->page_id = cache_bridge_next_page_id(slot);
        slot->binding_count += 1;
        return binding;
    }

    free(key_copy);
    return NULL;
}

static void cache_bridge_unbind_key(cache_bridge_slot_t *slot, cache_bridge_key_binding_t *binding) {
    if (!slot || !binding || !binding->in_use) {
        return;
    }

    cache_bridge_release_binding(binding);
    if (slot->binding_count > 0) {
        slot->binding_count -= 1;
    }
}

static const char *cache_bridge_handle_create(const char *params_json) {
    static char response[CACHE_BRIDGE_MAX_RESPONSE_LEN];
    cache_bridge_slot_t *slot;
    char cache_namespace[CACHE_BRIDGE_MAX_NAMESPACE_LEN];
    size_t capacity_pages = 0;
    size_t page_size = 0;

    if (cache_bridge_extract_string(params_json, "namespace", cache_namespace, sizeof(cache_namespace)) != 0 ||
        cache_bridge_extract_size(params_json, "capacityPages", &capacity_pages) != 0 ||
        cache_bridge_extract_size(params_json, "pageSize", &page_size) != 0) {
        return cache_bridge_error_response("invalid_create_request");
    }

    if (cache_bridge_validate_config(capacity_pages, page_size) != 0) {
        return cache_bridge_error_response("invalid_cache_config");
    }

    slot = cache_bridge_alloc_slot();
    if (!slot) {
        return cache_bridge_error_response("cache_registry_full");
    }

    snprintf(slot->handle, sizeof(slot->handle), "cache-%u", g_next_handle_id++);
    snprintf(slot->cache_namespace, sizeof(slot->cache_namespace), "%s", cache_namespace);

    if (cache_bridge_init_bindings(slot, capacity_pages) != 0) {
        memset(slot, 0, sizeof(*slot));
        return cache_bridge_error_response("cache_init_failed");
    }

    if (cache_service_init(&slot->service, capacity_pages, page_size) != 0) {
        cache_bridge_release_bindings(slot);
        memset(slot, 0, sizeof(*slot));
        return cache_bridge_error_response("cache_init_failed");
    }

    snprintf(response, sizeof(response),
             "{\"ok\":true,\"handle\":\"%s\"}",
             slot->handle);
    return response;
}

static const char *cache_bridge_handle_destroy(const char *params_json) {
    char handle[CACHE_BRIDGE_MAX_HANDLE_LEN];
    cache_bridge_slot_t *slot;

    if (cache_bridge_extract_string(params_json, "handle", handle, sizeof(handle)) != 0) {
        return cache_bridge_error_response("invalid_handle");
    }

    slot = cache_bridge_find_slot(handle);
    if (!slot) {
        return cache_bridge_error_response("cache_not_found");
    }

    cache_bridge_reset_slot(slot);
    return cache_bridge_ok_response();
}

static const char *cache_bridge_handle_get(const char *params_json) {
    static char response[CACHE_BRIDGE_MAX_RESPONSE_LEN];
    char handle[CACHE_BRIDGE_MAX_HANDLE_LEN];
    char key[CACHE_BRIDGE_MAX_KEY_LEN];
    char escaped[CACHE_BRIDGE_MAX_RESPONSE_LEN / 2];
    cache_bridge_slot_t *slot;
    uint8_t value[CACHE_BRIDGE_MAX_VALUE_LEN];
    size_t value_len = sizeof(value) - 1;

    if (cache_bridge_extract_string(params_json, "handle", handle, sizeof(handle)) != 0 ||
        cache_bridge_extract_string(params_json, "key", key, sizeof(key)) != 0) {
        return cache_bridge_error_response("invalid_get_request");
    }

    slot = cache_bridge_find_slot(handle);
    if (!slot) {
        return cache_bridge_error_response("cache_not_found");
    }

    if (cache_bridge_get_value(handle, key, value, &value_len) != 0) {
        return "{\"ok\":true,\"hit\":false}";
    }

    value[value_len] = '\0';
    cache_bridge_escape_json_string((const char *)value, escaped, sizeof(escaped));
    snprintf(response, sizeof(response),
             "{\"ok\":true,\"hit\":true,\"serializedValue\":\"%s\"}",
             escaped);
    return response;
}

static const char *cache_bridge_handle_set(const char *params_json) {
    char handle[CACHE_BRIDGE_MAX_HANDLE_LEN];
    char key[CACHE_BRIDGE_MAX_KEY_LEN];
    char serialized_value[CACHE_BRIDGE_MAX_VALUE_LEN];
    cache_bridge_slot_t *slot;

    if (cache_bridge_extract_string(params_json, "handle", handle, sizeof(handle)) != 0 ||
        cache_bridge_extract_string(params_json, "key", key, sizeof(key)) != 0 ||
        cache_bridge_extract_string(params_json, "serializedValue", serialized_value, sizeof(serialized_value)) != 0) {
        return cache_bridge_error_response("invalid_set_request");
    }

    slot = cache_bridge_find_slot(handle);
    if (!slot) {
        return cache_bridge_error_response("cache_not_found");
    }

    if (cache_bridge_set_value(handle,
                               key,
                               (const uint8_t *)serialized_value,
                               strlen(serialized_value)) != 0) {
        return cache_bridge_error_response("cache_set_failed");
    }

    return cache_bridge_ok_response();
}

static const char *cache_bridge_handle_has(const char *params_json) {
    static char response[CACHE_BRIDGE_MAX_RESPONSE_LEN];
    char handle[CACHE_BRIDGE_MAX_HANDLE_LEN];
    char key[CACHE_BRIDGE_MAX_KEY_LEN];
    cache_bridge_slot_t *slot;
    int has_value;

    if (cache_bridge_extract_string(params_json, "handle", handle, sizeof(handle)) != 0 ||
        cache_bridge_extract_string(params_json, "key", key, sizeof(key)) != 0) {
        return cache_bridge_error_response("invalid_has_request");
    }

    slot = cache_bridge_find_slot(handle);
    if (!slot) {
        return cache_bridge_error_response("cache_not_found");
    }

    has_value = cache_bridge_has_value(handle, key);
    snprintf(response, sizeof(response),
             "{\"ok\":true,\"has\":%s}",
             has_value ? "true" : "false");
    return response;
}

static const char *cache_bridge_handle_clear(const char *params_json) {
    char handle[CACHE_BRIDGE_MAX_HANDLE_LEN];
    cache_bridge_slot_t *slot;

    if (cache_bridge_extract_string(params_json, "handle", handle, sizeof(handle)) != 0) {
        return cache_bridge_error_response("invalid_handle");
    }

    slot = cache_bridge_find_slot(handle);
    if (!slot) {
        return cache_bridge_error_response("cache_not_found");
    }

    if (cache_bridge_clear_service(handle) != 0) {
        return cache_bridge_error_response("cache_clear_failed");
    }

    return cache_bridge_ok_response();
}

static const char *cache_bridge_handle_pin(const char *params_json) {
    char handle[CACHE_BRIDGE_MAX_HANDLE_LEN];
    char key[CACHE_BRIDGE_MAX_KEY_LEN];
    cache_bridge_slot_t *slot;

    if (cache_bridge_extract_string(params_json, "handle", handle, sizeof(handle)) != 0 ||
        cache_bridge_extract_string(params_json, "key", key, sizeof(key)) != 0) {
        return cache_bridge_error_response("invalid_pin_request");
    }

    slot = cache_bridge_find_slot(handle);
    if (!slot) {
        return cache_bridge_error_response("cache_not_found");
    }

    if (cache_bridge_pin_value(handle, key) != 0) {
        return cache_bridge_error_response("cache_pin_failed");
    }

    return cache_bridge_ok_response();
}

static const char *cache_bridge_handle_release(const char *params_json) {
    char handle[CACHE_BRIDGE_MAX_HANDLE_LEN];
    char key[CACHE_BRIDGE_MAX_KEY_LEN];
    cache_bridge_slot_t *slot;

    if (cache_bridge_extract_string(params_json, "handle", handle, sizeof(handle)) != 0 ||
        cache_bridge_extract_string(params_json, "key", key, sizeof(key)) != 0) {
        return cache_bridge_error_response("invalid_release_request");
    }

    slot = cache_bridge_find_slot(handle);
    if (!slot) {
        return cache_bridge_error_response("cache_not_found");
    }

    if (cache_bridge_release_value(handle, key) != 0) {
        return cache_bridge_error_response("cache_release_failed");
    }

    return cache_bridge_ok_response();
}

static const char *cache_bridge_handle_stats(const char *params_json) {
    static char response[CACHE_BRIDGE_MAX_RESPONSE_LEN];
    char handle[CACHE_BRIDGE_MAX_HANDLE_LEN];
    cache_bridge_slot_t *slot;
    cache_service_stats_t stats;

    if (cache_bridge_extract_string(params_json, "handle", handle, sizeof(handle)) != 0) {
        return cache_bridge_error_response("invalid_handle");
    }

    slot = cache_bridge_find_slot(handle);
    if (!slot) {
        return cache_bridge_error_response("cache_not_found");
    }

    if (cache_service_stats(&slot->service, &stats) != 0) {
        return cache_bridge_error_response("cache_stats_failed");
    }

    snprintf(response, sizeof(response),
             "{\"ok\":true,\"entries\":%zu,\"implementation\":\"native_page_cache\","
             "\"hits\":%llu,\"misses\":%llu,\"evictions\":%llu,"
             "\"reservedBytes\":%zu,\"metadataBytes\":%zu,"
             "\"hashIndexBytes\":%zu,\"payloadCapacityBytes\":%zu}",
             stats.entries,
             (unsigned long long)stats.hits,
             (unsigned long long)stats.misses,
             (unsigned long long)stats.evictions,
             stats.reserved_bytes,
             stats.metadata_bytes,
             stats.hash_index_bytes,
             stats.payload_capacity_bytes);
    return response;
}

const char *cache_bridge_invoke(const char *method, const char *params_json) {
    if (method == NULL || params_json == NULL) {
        return cache_bridge_error_response("invalid_request");
    }

    if (strcmp(method, "cache.create") == 0) {
        return cache_bridge_handle_create(params_json);
    }
    if (strcmp(method, "cache.destroy") == 0) {
        return cache_bridge_handle_destroy(params_json);
    }
    if (strcmp(method, "cache.get") == 0) {
        return cache_bridge_handle_get(params_json);
    }
    if (strcmp(method, "cache.set") == 0) {
        return cache_bridge_handle_set(params_json);
    }
    if (strcmp(method, "cache.has") == 0) {
        return cache_bridge_handle_has(params_json);
    }
    if (strcmp(method, "cache.clear") == 0) {
        return cache_bridge_handle_clear(params_json);
    }
    if (strcmp(method, "cache.pin") == 0) {
        return cache_bridge_handle_pin(params_json);
    }
    if (strcmp(method, "cache.release") == 0) {
        return cache_bridge_handle_release(params_json);
    }
    if (strcmp(method, "cache.stats") == 0) {
        return cache_bridge_handle_stats(params_json);
    }

    return cache_bridge_error_response("unsupported_method");
}

int cache_bridge_get_value(const char *handle, const char *key, uint8_t *out_buffer, size_t *inout_len) {
    cache_bridge_slot_t *slot = cache_bridge_find_slot(handle);
    cache_bridge_key_binding_t *binding;

    if (!slot || !key) {
        return -1;
    }

    binding = cache_bridge_find_binding(slot, key);
    if (!binding) {
        return -1;
    }

    if (cache_service_get(&slot->service, binding->page_id, out_buffer, inout_len) != 0) {
        cache_bridge_unbind_key(slot, binding);
        return -1;
    }

    return 0;
}

int cache_bridge_get_value_ptr(const char *handle, const char *key, const uint8_t **out_data, size_t *out_len) {
    cache_bridge_slot_t *slot = cache_bridge_find_slot(handle);
    cache_bridge_key_binding_t *binding;

    if (!slot || !key) {
        return -1;
    }

    binding = cache_bridge_find_binding(slot, key);
    if (!binding) {
        return -1;
    }

    if (cache_service_get_ptr(&slot->service, binding->page_id, out_data, out_len) != 0) {
        cache_bridge_unbind_key(slot, binding);
        return -1;
    }

    return 0;
}

int cache_bridge_set_value(const char *handle, const char *key, const uint8_t *data, size_t data_len) {
    cache_bridge_slot_t *slot = cache_bridge_find_slot(handle);
    cache_bridge_key_binding_t *binding;
    int created = 0;

    if (!slot || !key) {
        return -1;
    }

    binding = cache_bridge_find_binding(slot, key);
    if (!binding) {
        binding = cache_bridge_bind_key(slot, key);
        created = 1;
    }

    if (!binding) {
        return -1;
    }

    if (cache_service_set(&slot->service, binding->page_id, data, data_len) != 0) {
        if (created) {
            cache_bridge_unbind_key(slot, binding);
        }
        return -1;
    }

    cache_bridge_compact_bindings(slot);
    return 0;
}

int cache_bridge_has_value(const char *handle, const char *key) {
    cache_bridge_slot_t *slot = cache_bridge_find_slot(handle);
    cache_bridge_key_binding_t *binding;

    if (!slot || !key) {
        return 0;
    }

    binding = cache_bridge_find_binding(slot, key);
    if (!binding) {
        return 0;
    }

    return cache_service_has(&slot->service, binding->page_id);
}

int cache_bridge_pin_value(const char *handle, const char *key) {
    cache_bridge_slot_t *slot = cache_bridge_find_slot(handle);
    cache_bridge_key_binding_t *binding;

    if (!slot || !key) {
        return -1;
    }

    binding = cache_bridge_find_binding(slot, key);
    if (!binding) {
        return -1;
    }

    if (cache_service_pin(&slot->service, binding->page_id) != 0) {
        if (!cache_service_has(&slot->service, binding->page_id)) {
            cache_bridge_unbind_key(slot, binding);
        }
        return -1;
    }

    return 0;
}

int cache_bridge_release_value(const char *handle, const char *key) {
    cache_bridge_slot_t *slot = cache_bridge_find_slot(handle);
    cache_bridge_key_binding_t *binding;

    if (!slot || !key) {
        return -1;
    }

    binding = cache_bridge_find_binding(slot, key);
    if (!binding) {
        return -1;
    }

    if (cache_service_release(&slot->service, binding->page_id) != 0) {
        if (!cache_service_has(&slot->service, binding->page_id)) {
            cache_bridge_unbind_key(slot, binding);
        }
        return -1;
    }

    return 0;
}

cache_service_t *cache_bridge_get_service(const char *handle) {
    cache_bridge_slot_t *slot = cache_bridge_find_slot(handle);
    return slot ? &slot->service : NULL;
}

const char *cache_bridge_create_service(const char *namespace_name, size_t capacity_pages, size_t page_size) {
    cache_bridge_slot_t *slot;
    if (cache_bridge_validate_config(capacity_pages, page_size) != 0) {
        return NULL;
    }

    slot = cache_bridge_alloc_slot();
    if (!slot) {
        return NULL;
    }

    snprintf(slot->handle, sizeof(slot->handle), "cache-%u", g_next_handle_id++);
    if (namespace_name) {
        snprintf(slot->cache_namespace, sizeof(slot->cache_namespace), "%s", namespace_name);
    } else {
        slot->cache_namespace[0] = '\0';
    }

    if (cache_bridge_init_bindings(slot, capacity_pages) != 0) {
        memset(slot, 0, sizeof(*slot));
        return NULL;
    }

    if (cache_service_init(&slot->service, capacity_pages, page_size) != 0) {
        cache_bridge_release_bindings(slot);
        memset(slot, 0, sizeof(*slot));
        return NULL;
    }

    return slot->handle;
}

int cache_bridge_clear_service(const char *handle) {
    cache_bridge_slot_t *slot = cache_bridge_find_slot(handle);

    if (!slot) {
        return -1;
    }

    if (cache_service_clear(&slot->service) != 0) {
        return -1;
    }

    cache_bridge_release_bindings(slot);
    return cache_bridge_init_bindings(slot, slot->service.capacity_pages);
}

int cache_bridge_destroy_service(const char *handle) {
    cache_bridge_slot_t *slot = cache_bridge_find_slot(handle);
    if (!slot) {
        return -1;
    }

    cache_bridge_reset_slot(slot);
    return 0;
}

uint32_t cache_bridge_hash(const char *key) {
    return cache_bridge_hash_key(key);
}
