#ifndef CACHE_BRIDGE_H
#define CACHE_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "service.h"

/*
 * Vela/QuickJS bridge contract for the native cache service.
 *
 * This exports both a legacy JSON-based invoke and direct C APIs 
 * for more efficient native integration (Direct Provider).
 */

const char *cache_bridge_invoke(const char *method, const char *params_json);

cache_service_t *cache_bridge_get_service(const char *handle);
const char *cache_bridge_create_service(const char *namespace_name, size_t capacity_pages, size_t page_size);
int cache_bridge_clear_service(const char *handle);
int cache_bridge_destroy_service(const char *handle);
int cache_bridge_get_value(const char *handle, const char *key, uint8_t *out_buffer, size_t *inout_len);
int cache_bridge_get_value_ptr(const char *handle, const char *key, const uint8_t **out_data, size_t *out_len);
int cache_bridge_set_value(const char *handle, const char *key, const uint8_t *data, size_t data_len);
int cache_bridge_has_value(const char *handle, const char *key);
int cache_bridge_pin_value(const char *handle, const char *key);
int cache_bridge_release_value(const char *handle, const char *key);
uint32_t cache_bridge_hash(const char *key);

#ifdef __cplusplus
}
#endif

#endif
