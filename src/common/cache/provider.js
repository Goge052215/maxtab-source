import { hasProvider, registerProvider } from './bridge.js'

/**
 * Vela-friendly native cache provider stub.
 *
 * The runtime can inject either:
 * - a direct provider object on a global key, or
 * - a bridge object with `invoke(method, paramsJson)`
 *
 * This avoids dynamic native loading in JS and keeps the app-side API stable.
 */

const GLOBAL_PROVIDER_KEYS = [
  '__maxtabCacheProvider',
  '__velaCacheProvider',
  '__maxtabCacheBridge',
  '__velaCacheBridge',
]

function getGlobalScope() {
  if (typeof globalThis !== 'undefined') {
    return globalThis
  }
  if (typeof global !== 'undefined') {
    return global
  }
  if (typeof window !== 'undefined') {
    return window
  }
  return null
}

/** @param {any} candidate */
function isDirectProvider(candidate) {
  return !!(
    candidate &&
    typeof candidate.createCache === 'function' &&
    typeof candidate.get === 'function' &&
    typeof candidate.set === 'function'
  )
}

/** @param {any} candidate */
function isInvokeBridge(candidate) {
  return !!(candidate && typeof candidate.invoke === 'function')
}

/** @param {{ invoke: (method: string, paramsJson: string) => any }} methods */
function createInvokeBridgeProvider(methods) {
  /** @param {string} method @param {object} params */
  const invoke = (method, params) => {
    try {
      const rawResult = methods.invoke(method, JSON.stringify(params || {}))
      if (!rawResult) {
        return null
      }
      if (typeof rawResult === 'string') {
        return JSON.parse(rawResult)
      }
      return rawResult
    } catch (error) {
      return {
        ok: false,
        errorMessage: error && error.message ? error.message : 'cache_bridge_error',
      }
    }
  }

  return {
    /** @param {{ namespace: string, capacityPages: number, pageSize: number }} config */
    createCache(config) {
      const result = invoke('cache.create', config)
      return result && result.ok && typeof result.handle === 'string' ? result.handle : ''
    },
    /** @param {string} handle */
    destroyCache(handle) {
      invoke('cache.destroy', { handle })
    },
    /** @param {string} handle @param {string} key */
    get(handle, key) {
      const result = invoke('cache.get', { handle, key })
      return {
        hit: !!(result && result.ok && result.hit),
        serializedValue:
          result && typeof result.serializedValue === 'string' ? result.serializedValue : null,
      }
    },
    /** @param {string} handle @param {string} key @param {string} serializedValue */
    set(handle, key, serializedValue) {
      invoke('cache.set', { handle, key, serializedValue })
    },
    /** @param {string} handle @param {string} key */
    has(handle, key) {
      const result = invoke('cache.has', { handle, key })
      return !!(result && result.ok && result.has)
    },
    /** @param {string} handle */
    clear(handle) {
      invoke('cache.clear', { handle })
    },
    /** @param {string} handle @param {string} key */
    pin(handle, key) {
      invoke('cache.pin', { handle, key })
    },
    /** @param {string} handle @param {string} key */
    release(handle, key) {
      invoke('cache.release', { handle, key })
    },
    /** @param {string} handle */
    stats(handle) {
      const result = invoke('cache.stats', { handle })
      if (!result || !result.ok) {
        return {
          implementation: 'native_bridge_stub',
        }
      }
      return {
        entries: typeof result.entries === 'number' ? result.entries : 0,
        implementation: result.implementation || 'native_bridge_stub',
      }
    },
  }
}

function findInjectedProvider() {
  const scope = getGlobalScope()
  let i
  let candidate

  if (!scope) {
    return null
  }

  for (i = 0; i < GLOBAL_PROVIDER_KEYS.length; i += 1) {
    candidate = scope[GLOBAL_PROVIDER_KEYS[i]]
    if (isDirectProvider(candidate)) {
      return candidate
    }
    if (isInvokeBridge(candidate)) {
      return createInvokeBridgeProvider(candidate)
    }
  }

  return null
}

export function ensureVelaProvider() {
  if (hasProvider()) {
    return true
  }

  const injectedProvider = findInjectedProvider()
  if (!injectedProvider) {
    return false
  }

  registerProvider(injectedProvider)
  return true
}

/** @param {any} provider */
export function installVelaProvider(provider) {
  if (isDirectProvider(provider)) {
    registerProvider(provider)
    return true
  }

  if (isInvokeBridge(provider)) {
    registerProvider(createInvokeBridgeProvider(provider))
    return true
  }

  return false
}
