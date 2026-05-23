/**
 * @typedef {{
 *   createCache: (config: {
 *     namespace: string,
 *     capacityPages: number,
 *     pageSize: number
 *   }) => string,
 *   destroyCache?: (handle: string) => void,
 *   get: (handle: string, key: string) => { hit: boolean, serializedValue?: string | null },
 *   set: (handle: string, key: string, serializedValue: string) => void,
 *   has?: (handle: string, key: string) => boolean,
 *   clear?: (handle: string) => void,
 *   pin?: (handle: string, key: string) => void,
 *   release?: (handle: string, key: string) => void,
 *   stats?: (handle: string) => { entries?: number, implementation?: string }
 * }} CacheProvider
 */

/** @type {CacheProvider | null} */
let provider = null

/** @param {CacheProvider | null} nextProvider */
export function registerProvider(nextProvider) {
  provider = nextProvider
}

export function hasProvider() {
  return !!(
    provider &&
    typeof provider.createCache === 'function' &&
    typeof provider.get === 'function' &&
    typeof provider.set === 'function'
  )
}

/**
 * @param {{
 *   namespace?: string,
 *   capacityPages?: number,
 *   pageSize?: number,
 *   serialize?: (value: any) => string,
 *   deserialize?: (serialized: string) => any
 * }} options
 */
export function createBackend({
  namespace = 'default',
  capacityPages = 256,
  pageSize = 4096,
  serialize = JSON.stringify,
  deserialize = JSON.parse,
} = {}) {
  if (!hasProvider()) {
    return null
  }

  const handle = provider.createCache({
    namespace,
    capacityPages,
    pageSize,
  })

  if (!handle) {
    return null
  }

  return {
    /** @param {number|string} key */
    get(key) {
      const result = provider.get(handle, String(key))
      if (!result || !result.hit || typeof result.serializedValue !== 'string') {
        return null
      }
      return deserialize(result.serializedValue)
    },

    /** @param {number|string} key @param {any} value */
    set(key, value) {
      provider.set(handle, String(key), serialize(value))
    },

    /** @param {number|string} key */
    has(key) {
      if (typeof provider.has === 'function') {
        return provider.has(handle, String(key))
      }
      return this.get(key) !== null
    },

    clear() {
      if (typeof provider.clear === 'function') {
        provider.clear(handle)
      }
    },

    /** @param {number|string} key */
    pin(key) {
      if (typeof provider.pin === 'function') {
        provider.pin(handle, String(key))
      }
    },

    /** @param {number|string} key */
    release(key) {
      if (typeof provider.release === 'function') {
        provider.release(handle, String(key))
      }
    },

    destroy() {
      if (typeof provider.destroyCache === 'function') {
        provider.destroyCache(handle)
      }
    },

    stats() {
      const result = typeof provider.stats === 'function' ? provider.stats(handle) : {}
      return {
        implementation: 'native_bridge',
        ...(result || {}),
      }
    },

    get size() {
      const result = typeof provider.stats === 'function' ? provider.stats(handle) : null
      return result && typeof result.entries === 'number' ? result.entries : 0
    },
  }
}
