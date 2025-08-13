/**
 * Mock Native Module for testing C integration
 * This simulates the C calculation engine responses for development/testing
 */

/**
 * Mock implementation of the C calculation engine
 * Returns mathematically correct results using JavaScript but with C-like response structure
 */
class MockNativeModule {
  /**
   * Mock native function call
   * @param {string} functionName - Should be 'orchestrator_calculate_with_request'
   * @param {object} params - Calculation request parameters
   * @returns {object} - Calculation result in C format
   */
  static call(functionName, params) {
    if (functionName !== 'orchestrator_calculate_with_request') {
      return {
        success: 0,
        pdf_result: 0,
        cdf_result: 0,
        error_message: `Unknown function: ${functionName}`
      }
    }

    try {
      const { distribution, parameters, param_count, input_value } = params
      
      // Validate basic structure
      if (!parameters || !Array.isArray(parameters) || param_count !== parameters.length) {
        return {
          success: 0,
          pdf_result: 0,
          cdf_result: 0,
          error_message: 'Invalid parameter structure'
        }
      }

      // Route to appropriate distribution calculation
      let result
      switch (distribution) {
        case 7: // DIST_BINOMIAL
          result = this.calculateBinomialMock(parameters[0], parameters[1], input_value)
          break
        case 9: // DIST_POISSON
          result = this.calculatePoissonMock(parameters[0], input_value)
          break
        case 5: // DIST_GEOMETRIC
          result = this.calculateGeometricMock(parameters[0], input_value)
          break
        case 0: // DIST_NORMAL
          result = this.calculateNormalMock(parameters[0], parameters[1], input_value)
          break
        default:
          return {
            success: 0,
            pdf_result: 0,
            cdf_result: 0,
            error_message: `Unsupported distribution type: ${distribution}`
          }
      }

      return {
        success: 1,
        pdf_result: result.pdf,
        cdf_result: result.cdf,
        error_message: null
      }

    } catch (error) {
      return {
        success: 0,
        pdf_result: 0,
        cdf_result: 0,
        error_message: `Calculation error: ${error.message}`
      }
    }
  }

  /**
   * Mock binomial calculation using same math as C implementation
   */
  static calculateBinomialMock(n, p, k) {
    if (n <= 0 || p < 0 || p > 1 || k < 0 || k > n) {
      throw new Error('Invalid binomial parameters')
    }

    // Use log space for numerical stability (matches C implementation)
    const logCombo = this.logCombination(n, k)
    const logProb = logCombo + k * Math.log(p) + (n - k) * Math.log(1 - p)
    const pdf = Math.exp(logProb)
    
    // Calculate CDF
    let cdf = 0
    for (let i = 0; i <= k; i++) {
      const logComboI = this.logCombination(n, i)
      const logProbI = logComboI + i * Math.log(p) + (n - i) * Math.log(1 - p)
      cdf += Math.exp(logProbI)
    }
    
    return { pdf, cdf }
  }

  /**
   * Mock Poisson calculation
   */
  static calculatePoissonMock(lambda, k) {
    if (lambda <= 0 || k < 0) {
      throw new Error('Invalid Poisson parameters')
    }

    const logProb = k * Math.log(lambda) - lambda - this.logFactorial(k)
    const pdf = Math.exp(logProb)
    
    let cdf = 0
    for (let i = 0; i <= k; i++) {
      const logProbI = i * Math.log(lambda) - lambda - this.logFactorial(i)
      cdf += Math.exp(logProbI)
    }
    
    return { pdf, cdf }
  }

  /**
   * Mock geometric calculation
   */
  static calculateGeometricMock(p, k) {
    if (p <= 0 || p > 1 || k < 1) {
      throw new Error('Invalid geometric parameters')
    }

    const logProb = (k - 1) * Math.log(1 - p) + Math.log(p)
    const pdf = Math.exp(logProb)
    const cdf = 1.0 - Math.exp(k * Math.log(1 - p))
    
    return { pdf, cdf }
  }

  /**
   * Mock normal distribution calculation
   */
  static calculateNormalMock(mu, sigma, x) {
    if (sigma <= 0) {
      throw new Error('Invalid normal parameters')
    }

    const z = (x - mu) / sigma
    const pdf = (1 / (sigma * Math.sqrt(2 * Math.PI))) * Math.exp(-0.5 * z * z)
    const cdf = this.standardNormalCDF(z)
    
    return { pdf, cdf }
  }

  // Helper functions (same as in native_bridge.js)
  static logCombination(n, k) {
    if (k > n || k < 0) return -Infinity
    if (k === 0 || k === n) return 0
    return this.logFactorial(n) - this.logFactorial(k) - this.logFactorial(n - k)
  }

  static logFactorial(n) {
    if (n <= 1) return 0
    let result = 0
    for (let i = 2; i <= n; i++) {
      result += Math.log(i)
    }
    return result
  }

  static standardNormalCDF(z) {
    if (z < 0) return 1 - this.standardNormalCDF(-z)
    
    const t = 1 / (1 + 0.2316419 * z)
    const coefficients = [0.319381530, -0.356563782, 1.781477937, -1.821255978, 1.330274429]
    
    let polynomial = coefficients[0] * t
    for (let i = 1; i < coefficients.length; i++) {
      polynomial += coefficients[i] * Math.pow(t, i + 1)
    }
    
    const pdf = (1 / Math.sqrt(2 * Math.PI)) * Math.exp(-0.5 * z * z)
    return 1 - pdf * polynomial
  }
}

export { MockNativeModule }
