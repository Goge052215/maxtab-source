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
  }

  getKeyboardResult() {
    const result = this.keyboardResult
    
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

  _clearCalculatorResultInternal() {
    if (this.calculatorImport && typeof this.calculatorImport === 'object') {
      this._activeReferences.delete(this.calculatorImport)
    }
    if (this.calculatorExport && typeof this.calculatorExport === 'object') {
      this._activeReferences.delete(this.calculatorExport)
    }
    this.calculatorImport = null
    this.calculatorExport = null
    this.calculatorTimestamp = null
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
        this.forceCleanup()
      })
      
      memoryMonitor.onMemoryCritical(() => {
        this._performAggressiveCleanup()
      })
    } catch (error) {
      // Memory monitoring setup failed
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
      
      // Force immediate cleanup
      this._performMemoryCleanup()
    } catch (error) {
      // Aggressive cleanup error
    }
  }

}

export const globalState = new GlobalState()
