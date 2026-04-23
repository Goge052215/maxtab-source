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

interface ComponentBuildinType<T> {
  $forceUpdate: () => void
  $page: {
    name: string
    path: string
    component: string
    setTitleBar: (options: { text: string }) => void
  }
}

declare module '@system.configuration' {
  const configuration: {
    getLocale: () => { language?: string } | null
    setLocale: (options: { language: string; countryOrRegion: string }) => void
  }
  export default configuration
}

declare module '@system.storage' {
  const storage: {
    get: (options: {
      key: string
      success?: (data: any) => void
      fail?: (error?: any) => void
    }) => void
    set: (options: {
      key: string
      value: any
      success?: () => void
      fail?: (error?: any) => void
    }) => void
  }
  export default storage
}

declare module '*.json' {
  const value: any
  export default value
}
