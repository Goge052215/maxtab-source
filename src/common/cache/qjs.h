#ifndef CACHE_QJS_H
#define CACHE_QJS_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__has_include)
#  if __has_include("quickjs.h")
#    include "quickjs.h"
#    define CACHE_HAS_QUICKJS 1
#  endif
#endif

#ifndef CACHE_HAS_QUICKJS
typedef struct JSContext JSContext;
#endif

int cache_register_quickjs(JSContext *ctx);

#ifdef __cplusplus
}
#endif

#endif
