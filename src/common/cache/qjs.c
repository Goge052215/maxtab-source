#include "qjs.h"
#include "bridge.h"
#include "service.h"
#include <string.h>

#if defined(CACHE_HAS_QUICKJS)

enum {
    QJS_CACHE_MAX_CAPACITY_PAGES = 1024,
    QJS_CACHE_MAX_PAGE_SIZE = 4096
};

static int get_prop_int(JSContext *ctx, JSValue obj, const char *prop, int default_val) {
    JSValue val = JS_GetPropertyStr(ctx, obj, prop);
    if (JS_IsUndefined(val) || JS_IsException(val)) {
        JS_FreeValue(ctx, val);
        return default_val;
    }
    int32_t res;
    if (JS_ToInt32(ctx, &res, val) < 0) {
        JS_FreeValue(ctx, val);
        return default_val;
    }
    JS_FreeValue(ctx, val);
    return res;
}

static const char *get_prop_str(JSContext *ctx, JSValue obj, const char *prop) {
    JSValue val = JS_GetPropertyStr(ctx, obj, prop);
    if (JS_IsUndefined(val) || JS_IsException(val)) {
        JS_FreeValue(ctx, val);
        return NULL;
    }
    const char *str = JS_ToCString(ctx, val);
    JS_FreeValue(ctx, val);
    return str;
}

static int is_valid_cache_config(int capacity_pages, int page_size) {
    return capacity_pages > 0 &&
           page_size > 0 &&
           capacity_pages <= QJS_CACHE_MAX_CAPACITY_PAGES &&
           page_size <= QJS_CACHE_MAX_PAGE_SIZE;
}

static JSValue qjs_cache_create(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    const char *namespace_name;
    int capacity_pages;
    int page_size;
    const char *handle;

    (void)this_val;

    if (argc < 1 || !JS_IsObject(argv[0])) {
        return JS_ThrowTypeError(ctx, "Expected config object");
    }

    namespace_name = get_prop_str(ctx, argv[0], "namespace");
    capacity_pages = get_prop_int(ctx, argv[0], "capacityPages", 256);
    page_size = get_prop_int(ctx, argv[0], "pageSize", 4096);

    if (!is_valid_cache_config(capacity_pages, page_size)) {
        if (namespace_name) {
            JS_FreeCString(ctx, namespace_name);
        }
        return JS_ThrowRangeError(ctx, "Invalid cache size configuration");
    }

    handle = cache_bridge_create_service(
        namespace_name ? namespace_name : "default",
        (size_t)capacity_pages,
        (size_t)page_size
    );

    if (namespace_name) {
        JS_FreeCString(ctx, namespace_name);
    }

    if (!handle) {
        return JS_NULL;
    }
    return JS_NewString(ctx, handle);
}

static JSValue qjs_cache_destroy(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    const char *handle;

    (void)this_val;

    if (argc < 1) {
        return JS_UNDEFINED;
    }

    handle = JS_ToCString(ctx, argv[0]);
    if (handle) {
        cache_bridge_destroy_service(handle);
        JS_FreeCString(ctx, handle);
    }
    return JS_UNDEFINED;
}

static JSValue qjs_cache_get(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    const char *handle;
    const char *key;
    JSValue result;
    const uint8_t *data;
    size_t data_len;

    (void)this_val;

    if (argc < 2) {
        return JS_ThrowTypeError(ctx, "Expected handle and key");
    }

    handle = JS_ToCString(ctx, argv[0]);
    key = JS_ToCString(ctx, argv[1]);

    if (!handle || !key) {
        if (handle) JS_FreeCString(ctx, handle);
        if (key) JS_FreeCString(ctx, key);
        return JS_EXCEPTION;
    }

    result = JS_NewObject(ctx);
    if (cache_bridge_get_value_ptr(handle, key, &data, &data_len) == 0) {
        JS_SetPropertyStr(ctx, result, "hit", JS_NewBool(ctx, 1));
        JS_SetPropertyStr(ctx, result, "serializedValue", JS_NewStringLen(ctx, (const char *)data, data_len));
    } else {
        JS_SetPropertyStr(ctx, result, "hit", JS_NewBool(ctx, 0));
    }

    JS_FreeCString(ctx, handle);
    JS_FreeCString(ctx, key);
    return result;
}

static JSValue qjs_cache_set(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    const char *handle;
    const char *key;
    size_t val_len;
    const char *val_str;
    (void)this_val;

    if (argc < 3) {
        return JS_ThrowTypeError(ctx, "Expected handle, key, value");
    }

    handle = JS_ToCString(ctx, argv[0]);
    key = JS_ToCString(ctx, argv[1]);
    val_str = JS_ToCStringLen(ctx, &val_len, argv[2]);

    if (!handle || !key || !val_str) {
        if (handle) JS_FreeCString(ctx, handle);
        if (key) JS_FreeCString(ctx, key);
        if (val_str) JS_FreeCString(ctx, val_str);
        return JS_EXCEPTION;
    }

    cache_bridge_set_value(handle, key, (const uint8_t *)val_str, val_len);

    JS_FreeCString(ctx, handle);
    JS_FreeCString(ctx, key);
    JS_FreeCString(ctx, val_str);

    return JS_UNDEFINED;
}

static JSValue qjs_cache_has(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    const char *handle;
    const char *key;
    int has_val = 0;

    (void)this_val;

    if (argc < 2) {
        return JS_NewBool(ctx, 0);
    }

    handle = JS_ToCString(ctx, argv[0]);
    key = JS_ToCString(ctx, argv[1]);

    if (handle && key) {
        has_val = cache_bridge_has_value(handle, key);
    }

    if (handle) JS_FreeCString(ctx, handle);
    if (key) JS_FreeCString(ctx, key);

    return JS_NewBool(ctx, has_val);
}

static JSValue qjs_cache_clear(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    const char *handle;

    (void)this_val;

    if (argc < 1) {
        return JS_UNDEFINED;
    }

    handle = JS_ToCString(ctx, argv[0]);
    if (handle) {
        cache_bridge_clear_service(handle);
        JS_FreeCString(ctx, handle);
    }
    return JS_UNDEFINED;
}

static JSValue qjs_cache_pin(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    const char *handle;
    const char *key;

    (void)this_val;

    if (argc < 2) {
        return JS_UNDEFINED;
    }

    handle = JS_ToCString(ctx, argv[0]);
    key = JS_ToCString(ctx, argv[1]);

    if (handle && key) {
        cache_bridge_pin_value(handle, key);
    }

    if (handle) JS_FreeCString(ctx, handle);
    if (key) JS_FreeCString(ctx, key);
    return JS_UNDEFINED;
}

static JSValue qjs_cache_release(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    const char *handle;
    const char *key;

    (void)this_val;

    if (argc < 2) {
        return JS_UNDEFINED;
    }

    handle = JS_ToCString(ctx, argv[0]);
    key = JS_ToCString(ctx, argv[1]);

    if (handle && key) {
        cache_bridge_release_value(handle, key);
    }

    if (handle) JS_FreeCString(ctx, handle);
    if (key) JS_FreeCString(ctx, key);
    return JS_UNDEFINED;
}

static JSValue qjs_cache_stats(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    JSValue result;
    const char *handle;

    (void)this_val;

    result = JS_NewObject(ctx);
    if (argc < 1) {
        return result;
    }

    handle = JS_ToCString(ctx, argv[0]);
    if (handle) {
        cache_service_t *svc = cache_bridge_get_service(handle);
        cache_service_stats_t stats;
        if (svc && cache_service_stats(svc, &stats) == 0) {
            JS_SetPropertyStr(ctx, result, "entries", JS_NewInt32(ctx, (int32_t)stats.entries));
            JS_SetPropertyStr(ctx, result, "implementation", JS_NewString(ctx, "native_provider"));
            JS_SetPropertyStr(ctx, result, "hits", JS_NewInt64(ctx, (int64_t)stats.hits));
            JS_SetPropertyStr(ctx, result, "misses", JS_NewInt64(ctx, (int64_t)stats.misses));
            JS_SetPropertyStr(ctx, result, "evictions", JS_NewInt64(ctx, (int64_t)stats.evictions));
        }
        JS_FreeCString(ctx, handle);
    }
    return result;
}

int cache_register_quickjs(JSContext *ctx) {
    JSValue global_obj;
    JSValue provider_obj;

    if (ctx == NULL) {
        return -1;
    }

    global_obj = JS_GetGlobalObject(ctx);
    provider_obj = JS_NewObject(ctx);

    if (JS_IsException(provider_obj) || JS_IsException(global_obj)) {
        JS_FreeValue(ctx, provider_obj);
        JS_FreeValue(ctx, global_obj);
        return -1;
    }

    JS_SetPropertyStr(ctx, provider_obj, "createCache", JS_NewCFunction(ctx, qjs_cache_create, "createCache", 1));
    JS_SetPropertyStr(ctx, provider_obj, "destroyCache", JS_NewCFunction(ctx, qjs_cache_destroy, "destroyCache", 1));
    JS_SetPropertyStr(ctx, provider_obj, "get", JS_NewCFunction(ctx, qjs_cache_get, "get", 2));
    JS_SetPropertyStr(ctx, provider_obj, "set", JS_NewCFunction(ctx, qjs_cache_set, "set", 3));
    JS_SetPropertyStr(ctx, provider_obj, "has", JS_NewCFunction(ctx, qjs_cache_has, "has", 2));
    JS_SetPropertyStr(ctx, provider_obj, "clear", JS_NewCFunction(ctx, qjs_cache_clear, "clear", 1));
    JS_SetPropertyStr(ctx, provider_obj, "pin", JS_NewCFunction(ctx, qjs_cache_pin, "pin", 2));
    JS_SetPropertyStr(ctx, provider_obj, "release", JS_NewCFunction(ctx, qjs_cache_release, "release", 2));
    JS_SetPropertyStr(ctx, provider_obj, "stats", JS_NewCFunction(ctx, qjs_cache_stats, "stats", 1));

    if (JS_SetPropertyStr(ctx, global_obj, "__velaCacheProvider", provider_obj) < 0) {
        JS_FreeValue(ctx, provider_obj);
        JS_FreeValue(ctx, global_obj);
        return -1;
    }

    JS_FreeValue(ctx, global_obj);
    return 0;
}

#else

int cache_register_quickjs(JSContext *ctx) {
    (void)ctx;
    return -1;
}

#endif
