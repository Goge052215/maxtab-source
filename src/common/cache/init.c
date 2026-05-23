#include "init.h"

#if defined(CACHE_HAS_QUICKJS)

static int cache_module_init(JSContext *ctx, JSModuleDef *module_def) {
    int rc;

    rc = cache_register_quickjs(ctx);
    if (rc != 0) {
        return rc;
    }

    /*
     * Export a tiny marker so native-module loading can verify that the
     * cache bridge was initialized successfully.
     */
    return JS_AddModuleExport(ctx, module_def, "ready");
}

int cache_plugin_init(JSContext *ctx) {
    return cache_register_quickjs(ctx);
}

/*
 * QuickJS-native module entry point.
 *
 * This gives two integration options:
 * 1. Vela firmware startup path calls `cache_plugin_init(ctx)` directly.
 * 2. A QuickJS module loader loads this shared object and calls
 *    `js_init_module()`, which also installs the global bridge.
 */
JS_MODULE_EXTERN JSModuleDef *js_init_module(JSContext *ctx, const char *module_name) {
    JSModuleDef *module_def;

    if (ctx == NULL || module_name == NULL) {
        return NULL;
    }

    module_def = JS_NewCModule(ctx, module_name, cache_module_init);
    if (!module_def) {
        return NULL;
    }

    if (JS_AddModuleExport(ctx, module_def, "ready") < 0) {
        return NULL;
    }

    return module_def;
}

#else

int cache_plugin_init(JSContext *ctx) {
    (void)ctx;
    return -1;
}

#endif
