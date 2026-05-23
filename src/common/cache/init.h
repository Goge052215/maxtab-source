#ifndef CACHE_INIT_H
#define CACHE_INIT_H

#include "qjs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Firmware-facing startup hook.
 *
 * Call this once after the QuickJS context is created for the app runtime.
 * It registers `globalThis.__velaCacheProvider` so the JS-side
 * provider in `provider.js` can bind to the native cache automatically.
 */
int cache_plugin_init(JSContext *ctx);

#ifdef __cplusplus
}
#endif

#endif
