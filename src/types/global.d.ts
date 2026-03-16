interface PerformanceMemory {
  jsHeapSizeLimit?: number
  totalJSHeapSize?: number
  usedJSHeapSize?: number
}

interface Performance {
  memory?: PerformanceMemory
}

interface Window {
  globalState?: {
    forceCleanup?: () => void
  }
  _tempDOMRefs?: unknown[] | { clear?: () => void }
}

declare module '@system.configuration' {
  const configuration: {
    getLocale: () => { language?: string } | null
  }
  export default configuration
}

declare module '*.json' {
  const value: any
  export default value
}
