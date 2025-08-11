// Simple global state manager for parameter passing
class GlobalState {
  constructor() {
    this.keyboardResult = null
    this.resultTimestamp = null
    // Calculator import/export bridge
    this.calculatorImport = null
    this.calculatorExport = null
    this.calculatorTimestamp = null
  }

  setKeyboardResult(result) {
    this.keyboardResult = result
    this.resultTimestamp = Date.now()
    console.log('GlobalState: Set keyboard result:', result)
  }

  getKeyboardResult() {
    const result = this.keyboardResult
    console.log('GlobalState: Get keyboard result:', result)
    
    // Only clear if result is older than 2 seconds to prevent race conditions
    if (result && this.resultTimestamp && (Date.now() - this.resultTimestamp > 2000)) {
      this.keyboardResult = null
      this.resultTimestamp = null
    } else if (result) {
      // Don't clear immediately, let multiple checks happen
      setTimeout(() => {
        this.keyboardResult = null
        this.resultTimestamp = null
      }, 1000)
    }
    
    return result
  }

  // Peek at result without clearing it
  peekKeyboardResult() {
    return this.keyboardResult
  }

  // Force clear result
  clearKeyboardResult() {
    this.keyboardResult = null
    this.resultTimestamp = null
  }

  // Calculator import/export helpers
  setCalculatorImport(importPayload) {
    this.calculatorImport = importPayload
    this.calculatorTimestamp = Date.now()
    console.log('GlobalState: Set calculator import:', importPayload)
  }

  getCalculatorImport() {
    return this.calculatorImport
  }

  clearCalculatorImport() {
    this.calculatorImport = null
  }

  setCalculatorExport(exportPayload) {
    this.calculatorExport = exportPayload
    this.calculatorTimestamp = Date.now()
    console.log('GlobalState: Set calculator export:', exportPayload)
  }

  getCalculatorExport() {
    const payload = this.calculatorExport
    if (payload && this.calculatorTimestamp && (Date.now() - this.calculatorTimestamp > 2000)) {
      this.calculatorExport = null
    } else if (payload) {
      setTimeout(() => {
        this.calculatorExport = null
      }, 1000)
    }
    return payload
  }

  clearCalculatorExport() {
    this.calculatorExport = null
  }
}

export const globalState = new GlobalState()