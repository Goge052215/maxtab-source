import LRUCache from '../lru_cache.js'
import { createBackend } from './bridge.js'
import { ensureVelaProvider } from './provider.js'

const createDestroyedBackend = () => ({
  get() {
    return null
  },
  set() {},
  has() {
    return false
  },
  clear() {},
  pin() {},
  release() {},
  destroy() {},
  stats() {
    return {
      implementation: 'destroyed_cache',
      entries: 0,
    }
  },
  get size() {
    return 0
  },
})

/** @param {number} capacityPages */
const createJsFallbackBackend = (capacityPages) => {
  const cache = new LRUCache(capacityPages)
  return {
    /** @param {number|string} key */
    get(key) {
      return cache.get(key)
    },
    /** @param {number|string} key @param {any} value */
    set(key, value) {
      cache.set(key, value)
    },
    /** @param {number|string} key */
    has(key) {
      return cache.has(key)
    },
    clear() {
      cache.clear()
    },
    pin() {},
    release() {},
    stats() {
      return {
        implementation: 'js_lru_fallback',
        entries: cache.size,
      }
    },
    get size() {
      return cache.size
    },
  }
}

/**
 * @param {{
 *   namespace: string,
 *   capacityPages: number,
 *   pageSize: number
 * }} options
 */
const createCacheBackend = ({ namespace, capacityPages, pageSize }) => {
  ensureVelaProvider()
  return (
    createBackend({
      namespace,
      capacityPages,
      pageSize,
    }) || createJsFallbackBackend(capacityPages)
  )
}

/**
 * Thin JS-facing cache API shape.
 * Replace the internals with a native bridge when the runtime binding is ready.
 */
class CacheService {
  constructor({ capacityPages = 256, pageSize = 4096, namespace = 'default' } = {}) {
    this.capacityPages = capacityPages
    this.pageSize = pageSize
    this.namespace = namespace
    this.backend = createCacheBackend({ namespace, capacityPages, pageSize })
    this.pinned = new Set()
    this.destroyed = false
  }

  configure({ capacityPages = this.capacityPages, pageSize = this.pageSize } = {}) {
    if (this.destroyed) {
      return
    }
    if (capacityPages !== this.capacityPages || pageSize !== this.pageSize) {
      this.capacityPages = capacityPages
      this.pageSize = pageSize
      if (typeof this.backend.destroy === 'function') {
        this.backend.destroy()
      }
      this.backend = createCacheBackend({
        namespace: this.namespace,
        capacityPages,
        pageSize,
      })
      this.pinned.clear()
      return
    }
  }

  /** @param {number|string} pageId */
  get(pageId) {
    return this.backend.get(pageId)
  }

  /** @param {number|string} pageId @param {any} value */
  set(pageId, value) {
    this.backend.set(pageId, value)
  }

  /** @param {number|string} pageId */
  has(pageId) {
    return this.backend.has(pageId)
  }

  clear() {
    this.backend.clear()
    this.pinned.clear()
  }

  destroy() {
    if (this.destroyed) {
      return
    }
    if (typeof this.backend.destroy === 'function') {
      this.backend.destroy()
    }
    this.backend = createDestroyedBackend()
    this.pinned.clear()
    this.destroyed = true
  }

  get size() {
    return this.backend.size
  }

  /** @param {number|string} pageId @param {(pageId: number|string) => any=} loader */
  getPage(pageId, loader) {
    const cached = this.get(pageId)
    if (cached) {
      return cached
    }

    const page = typeof loader === 'function' ? loader(pageId) : null
    if (page !== null && typeof page !== 'undefined') {
      this.set(pageId, page)
    }
    return page
  }

  /** @param {Array<number|string>} pageIds @param {(pageId: number|string) => any=} loader */
  prefetch(pageIds, loader) {
    if (!Array.isArray(pageIds) || typeof loader !== 'function') {
      return
    }

    pageIds.forEach((pageId) => {
      if (!this.has(pageId)) {
        const page = loader(pageId)
        if (page !== null && typeof page !== 'undefined') {
          this.set(pageId, page)
        }
      }
    })
  }

  /** @param {number|string} pageId @param {(pageId: number|string) => any=} loader */
  pin(pageId, loader) {
    const page = this.getPage(pageId, loader)
    if (page !== null && typeof page !== 'undefined') {
      this.backend.pin(pageId)
      this.pinned.add(pageId)
    }
    return page
  }

  /** @param {number|string} pageId */
  release(pageId) {
    this.backend.release(pageId)
    this.pinned.delete(pageId)
  }

  stats() {
    const backendStats = this.backend.stats()
    return {
      implementation: backendStats.implementation || 'js_thin_service_fallback',
      capacityPages: this.capacityPages,
      pageSize: this.pageSize,
      cachedPages: this.size,
      pinnedPages: this.pinned.size,
      ...backendStats,
    }
  }
}

export default CacheService
