import { memoryMonitor } from './memory_monitor.js'

// Global state manager for parameter passing
class GlobalState {
  constructor() {
    this.keyboardResult = null
    this.resultTimestamp = null
    this.calculatorImport = null
    this.calculatorExport = null
    this.calculatorTimestamp = null
    
    // Track object references for cleanup
    this._activeReferences = new Set()
    this._cleanupInterval = null
    this._maxRetentionTime = 5000 // 5 sec max retention
    
    this._startCleanupTimer()
    
    // Setup memory monitoring integration
    this._setupMemoryMonitoring()
  }

  // Start automatic cleanup timer
  _startCleanupTimer() {
    if (this._cleanupInterval) {
      clearInterval(this._cleanupInterval)
    }
    
    this._cleanupInterval = setInterval(() => {
      this._performMemoryCleanup()
    }, 2000) // Check every 2 seconds
  }

  // Aggressive cleanup of stale references
  _performMemoryCleanup() {
    const now = Date.now()
    
    // Clean keyboard results older than max retention time
    if (this.keyboardResult && this.resultTimestamp && 
        (now - this.resultTimestamp > this._maxRetentionTime)) {
      this.keyboardResult = null
      this.resultTimestamp = null
    }
    
    // Clean calculator data older than max retention time
    if (this.calculatorTimestamp && (now - this.calculatorTimestamp > this._maxRetentionTime)) {
      this.calculatorImport = null
      this.calculatorExport = null
      this.calculatorTimestamp = null
    }
    
    // Force garbage collection hint
    // ref https://www.tecmint.com/reduce-ram-cpu-usage-on-linux/
    if (typeof global !== 'undefined' && global.gc) {
      global.gc()
    }
  }

  setKeyboardResult(result) {
    // Clear previous result immediately
    if (this.keyboardResult && typeof this.keyboardResult === 'object') {
      this._activeReferences.delete(this.keyboardResult)
    }
    
    this.keyboardResult = result
    this.resultTimestamp = Date.now()
    
    if (result && typeof result === 'object') {
      this._activeReferences.add(result)
    }
    
    console.log('GlobalState: Set keyboard result:', result)
  }

  getKeyboardResult() {
    const result = this.keyboardResult
    console.log('GlobalState: Get keyboard result:', result)
    
    // Immediate clearing for memory efficiency
    // ref https://www.makeuseof.com/improve-performance-free-up-ram-on-linux/
    if (result && this.resultTimestamp && (Date.now() - this.resultTimestamp > 2000)) {
      this._clearKeyboardResultInternal()
    } else if (result) {
      setTimeout(() => {
        this._clearKeyboardResultInternal()
      }, 500) // Faster memory reclaim
    }
    
    return result
  }

  // Internal method for consistent cleanup
  _clearKeyboardResultInternal() {
    if (this.keyboardResult && typeof this.keyboardResult === 'object') {
      this._activeReferences.delete(this.keyboardResult)
    }
    this.keyboardResult = null
    this.resultTimestamp = null
  }

  // Peek at result without clearing it
  peekKeyboardResult() {
    return this.keyboardResult
  }

  // Force clear result with memory cleanup
  clearKeyboardResult() {
    this._clearKeyboardResultInternal()
  }

  // Calculator import/export helpers
  setCalculatorImport(importPayload) {
    // Clear previous import to free memory
    if (this.calculatorImport && typeof this.calculatorImport === 'object') {
      this._activeReferences.delete(this.calculatorImport)
    }
    
    this.calculatorImport = importPayload
    this.calculatorTimestamp = Date.now()
    
    if (importPayload && typeof importPayload === 'object') {
      this._activeReferences.add(importPayload)
    }
    
    console.log('GlobalState: Set calculator import:', importPayload)
  }

  getCalculatorImport() {
    return this.calculatorImport
  }

  clearCalculatorImport() {
    if (this.calculatorImport && typeof this.calculatorImport === 'object') {
      this._activeReferences.delete(this.calculatorImport)
    }
    this.calculatorImport = null
  }

  setCalculatorExport(exportPayload) {
    // Clear previous export to free memory
    if (this.calculatorExport && typeof this.calculatorExport === 'object') {
      this._activeReferences.delete(this.calculatorExport)
    }
    
    this.calculatorExport = exportPayload
    this.calculatorTimestamp = Date.now()
    
    if (exportPayload && typeof exportPayload === 'object') {
      this._activeReferences.add(exportPayload)
    }
    
    console.log('GlobalState: Set calculator export:', exportPayload)
  }

  getCalculatorExport() {
    const payload = this.calculatorExport
    
    // Optimized cleanup with shorter retention
    if (payload && this.calculatorTimestamp && (Date.now() - this.calculatorTimestamp > 2000)) {
      this.clearCalculatorExport()
    } else if (payload) {
      setTimeout(() => {
        this.clearCalculatorExport()
      }, 500) // Reduced from 1000ms
    }
    return payload
  }

  clearCalculatorExport() {
    if (this.calculatorExport && typeof this.calculatorExport === 'object') {
      this._activeReferences.delete(this.calculatorExport)
    }
    this.calculatorExport = null
  }

  // Manual cleanup method
  forceCleanup() {
    this._performMemoryCleanup()
    this._activeReferences.clear()
  }

  // Get memory usage statistics
  getMemoryStats() {
    return {
      activeReferences: this._activeReferences.size,
      hasKeyboardResult: !!this.keyboardResult,
      hasCalculatorImport: !!this.calculatorImport,
      hasCalculatorExport: !!this.calculatorExport,
      keyboardAge: this.resultTimestamp ? Date.now() - this.resultTimestamp : 0,
      calculatorAge: this.calculatorTimestamp ? Date.now() - this.calculatorTimestamp : 0
    }
  }

  // Cleanup on destruction
  destroy() {
    if (this._cleanupInterval) {
      clearInterval(this._cleanupInterval)
      this._cleanupInterval = null
    }
    this.forceCleanup()
  }

  // Memory optimization: Setup memory monitoring integration
  _setupMemoryMonitoring() {
    try {
      // Register cleanup callbacks with memory monitor
      memoryMonitor.onMemoryCleanup(() => {
        console.log('Memory monitor triggered global state cleanup')
        this.forceCleanup()
      })
      
      memoryMonitor.onMemoryCritical(() => {
        console.log('Critical memory - performing aggressive global state cleanup')
        this._performAggressiveCleanup()
      })
    } catch (error) {
      console.warn('Memory monitoring setup failed:', error)
    }
  }

  // Memory optimization: Aggressive cleanup for critical memory situations
  _performAggressiveCleanup() {
    try {
      // Clear all data immediately
      this._clearKeyboardResultInternal()
      this._clearCalculatorResultInternal()
      
      // Clear all active references
      this._activeReferences.clear()
      
      // Reset all state
      this._keyboardResult = null
      this._calculatorResult = null
      this._calculatorExport = null
      
      // Force immediate cleanup
      this._performMemoryCleanup()
      
      console.log('Aggressive global state cleanup completed')
    } catch (error) {
      console.error('Aggressive cleanup error:', error)
    }
  }

  // Memory optimization: Enhanced memory stats with monitoring integration
  getMemoryStats() {
    const baseStats = {
      keyboardResultSize: this._keyboardResult ? JSON.stringify(this._keyboardResult).length : 0,
      calculatorResultSize: this._calculatorResult ? JSON.stringify(this._calculatorResult).length : 0,
      calculatorExportSize: this._calculatorExport ? JSON.stringify(this._calculatorExport).length : 0,
      activeReferences: this._activeReferences.size,
      lastCleanup: this._lastCleanup,
      cleanupInterval: this._cleanupInterval ? 'active' : 'inactive'
    }
    
    // Add system memory stats if available
    try {
      const systemStats = memoryMonitor.getMemoryStats()
      return {
        ...baseStats,
        system: systemStats
      }
    } catch (error) {
      return baseStats
    }
  }
}

export const globalState = new GlobalState()