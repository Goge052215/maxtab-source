/**
 * Native Bridge for JavaScript-to-C function calls
 * Provides optimized statistical distribution calculations using C libraries
 */

// Import native C module (QuickApp framework specific)
import { NativeModule } from '@system.app'

/**
 * Distribution type constants matching C enum
 */
const DISTRIBUTION_TYPES = {
  DIST_NORMAL: 0,
  DIST_EXPONENTIAL: 1,
  DIST_CHI_SQUARE: 2,
  DIST_T_DISTRIBUTION: 3,
  DIST_F_DISTRIBUTION: 4,
  DIST_GEOMETRIC: 5,
  DIST_HYPERGEOMETRIC: 6,
  DIST_BINOMIAL: 7,
  DIST_NEGATIVE_BINOMIAL: 8,
  DIST_POISSON: 9
}

/**
 * Calculation result structure
 */
class CalculationResult {
  constructor(success = false, pdfResult = 0, cdfResult = 0, errorMessage = null) {
    this.success = success
    this.pdfResult = pdfResult
    this.cdfResult = cdfResult
    this.errorMessage = errorMessage
  }
}

/**
 * Native calculation bridge class
 */
class NativeBridge {
  
  /**
   * Calculate binomial distribution
   * @param {number} n - number of trials
   * @param {number} p - probability of success
   * @param {number} k - number of successes
   * @returns {CalculationResult}
   */
  static calculateBinomial(n, p, k) {
    try {
      const params = [n, p]
      const request = {
        distribution: DISTRIBUTION_TYPES.DIST_BINOMIAL,
        parameters: params,
        param_count: 2,
        input_value: k
      }
      
      // Call native C function through QuickApp bridge
      const result = NativeModule.call('orchestrator_calculate_with_request', request)
      
      return new CalculationResult(
        result.success === 1,
        result.pdf_result,
        result.cdf_result,
        result.error_message
      )
    } catch (error) {
      console.error('Native binomial calculation failed:', error)
      return this.fallbackBinomial(n, p, k)
    }
  }

  /**
   * Calculate Poisson distribution
   * @param {number} lambda - rate parameter
   * @param {number} k - number of events
   * @returns {CalculationResult}
   */
  static calculatePoisson(lambda, k) {
    try {
      const params = [lambda]
      const request = {
        distribution: DISTRIBUTION_TYPES.DIST_POISSON,
        parameters: params,
        param_count: 1,
        input_value: k
      }
      
      const result = NativeModule.call('orchestrator_calculate_with_request', request)
      
      return new CalculationResult(
        result.success === 1,
        result.pdf_result,
        result.cdf_result,
        result.error_message
      )
    } catch (error) {
      console.error('Native Poisson calculation failed:', error)
      return this.fallbackPoisson(lambda, k)
    }
  }

  /**
   * Calculate geometric distribution
   * @param {number} p - probability of success
   * @param {number} k - trial number of first success
   * @returns {CalculationResult}
   */
  static calculateGeometric(p, k) {
    try {
      const params = [p]
      const request = {
        distribution: DISTRIBUTION_TYPES.DIST_GEOMETRIC,
        parameters: params,
        param_count: 1,
        input_value: k
      }
      
      const result = NativeModule.call('orchestrator_calculate_with_request', request)
      
      return new CalculationResult(
        result.success === 1,
        result.pdf_result,
        result.cdf_result,
        result.error_message
      )
    } catch (error) {
      console.error('Native geometric calculation failed:', error)
      return this.fallbackGeometric(p, k)
    }
  }

  /**
   * Calculate negative binomial distribution
   * @param {number} r - number of successes
   * @param {number} p - probability of success
   * @param {number} k - number of failures
   * @returns {CalculationResult}
   */
  static calculateNegativeBinomial(r, p, k) {
    try {
      const params = [r, p]
      const request = {
        distribution: DISTRIBUTION_TYPES.DIST_NEGATIVE_BINOMIAL,
        parameters: params,
        param_count: 2,
        input_value: k
      }
      
      const result = NativeModule.call('orchestrator_calculate_with_request', request)
      
      return new CalculationResult(
        result.success === 1,
        result.pdf_result,
        result.cdf_result,
        result.error_message
      )
    } catch (error) {
      console.error('Native negative binomial calculation failed:', error)
      return this.fallbackNegativeBinomial(r, p, k)
    }
  }

  /**
   * Calculate normal distribution
   * @param {number} mu - mean parameter
   * @param {number} sigma - standard deviation parameter
   * @param {number} x - input value
   * @returns {CalculationResult}
   */
  static calculateNormalDistribution(mu, sigma, x) {
    try {
      const params = [mu, sigma]
      const request = {
        distribution: DISTRIBUTION_TYPES.DIST_NORMAL,
        parameters: params,
        param_count: 2,
        input_value: x
      }
      
      const result = NativeModule.call('orchestrator_calculate_with_request', request)
      
      return new CalculationResult(
        result.success === 1,
        result.pdf_result,
        result.cdf_result,
        result.error_message
      )
    } catch (error) {
      console.error('Native normal distribution calculation failed:', error)
      return this.fallbackNormalDistribution(mu, sigma, x)
    }
  }

  /**
   * Calculate hypergeometric distribution
   * @param {number} N - population size
   * @param {number} K - number of success items in population
   * @param {number} n - number of draws
   * @param {number} k - number of observed successes
   * @returns {CalculationResult}
   */
  static calculateHypergeometric(N, K, n, k) {
    try {
      const params = [N, K, n]
      const request = {
        distribution: DISTRIBUTION_TYPES.DIST_HYPERGEOMETRIC,
        parameters: params,
        param_count: 3,
        input_value: k
      }
      
      const result = NativeModule.call('orchestrator_calculate_with_request', request)
      
      return new CalculationResult(
        result.success === 1,
        result.pdf_result,
        result.cdf_result,
        result.error_message
      )
    } catch (error) {
      console.error('Native hypergeometric calculation failed:', error)
      return this.fallbackHypergeometric(N, K, n, k)
    }
  }

  // Fallback JavaScript implementations (simplified versions of previous code)
  
  static fallbackBinomial(n, p, k) {
    try {
      const logCombo = this.logCombination(n, k)
      const logProb = logCombo + k * Math.log(p) + (n - k) * Math.log(1 - p)
      const pmf = Math.exp(logProb)
      
      let cdf = 0
      for (let i = 0; i <= k; i++) {
        const logComboI = this.logCombination(n, i)
        const logProbI = logComboI + i * Math.log(p) + (n - i) * Math.log(1 - p)
        cdf += Math.exp(logProbI)
      }
      
      return new CalculationResult(true, pmf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'Fallback calculation failed')
    }
  }

  static fallbackPoisson(lambda, k) {
    try {
      const logProb = k * Math.log(lambda) - lambda - this.logFactorial(k)
      const pmf = Math.exp(logProb)
      
      let cdf = 0
      for (let i = 0; i <= k; i++) {
        const logProbI = i * Math.log(lambda) - lambda - this.logFactorial(i)
        cdf += Math.exp(logProbI)
      }
      
      return new CalculationResult(true, pmf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'Fallback calculation failed')
    }
  }

  static fallbackGeometric(p, k) {
    try {
      const logProb = (k - 1) * Math.log(1 - p) + Math.log(p)
      const pmf = Math.exp(logProb)
      const cdf = 1.0 - Math.exp(k * Math.log(1 - p))
      
      return new CalculationResult(true, pmf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'Fallback calculation failed')
    }
  }

  static fallbackNegativeBinomial(r, p, k) {
    try {
      const logCombo = this.logCombination(k + r - 1, k)
      const logProb = logCombo + r * Math.log(p) + k * Math.log(1 - p)
      const pmf = Math.exp(logProb)
      
      let cdf = 0
      for (let i = 0; i <= k; i++) {
        const logComboI = this.logCombination(i + r - 1, i)
        const logProbI = logComboI + r * Math.log(p) + i * Math.log(1 - p)
        cdf += Math.exp(logProbI)
      }
      
      return new CalculationResult(true, pmf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'Fallback calculation failed')
    }
  }

  static fallbackNormalDistribution(mu, sigma, x) {
    try {
      // Standard normal PDF calculation
      const z = (x - mu) / sigma
      const pdf = (1 / (sigma * Math.sqrt(2 * Math.PI))) * Math.exp(-0.5 * z * z)
      
      // Approximate CDF using Abramowitz and Stegun approximation
      const cdf = this.standardNormalCDF(z)
      
      return new CalculationResult(true, pdf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'Fallback calculation failed')
    }
  }

  static fallbackHypergeometric(N, K, n, k) {
    try {
      const logProb = this.logCombination(K, k) + 
                      this.logCombination(N - K, n - k) - 
                      this.logCombination(N, n)
      const pmf = Math.exp(logProb)
      
      const kMin = Math.max(0, n - (N - K))
      let cdf = 0
      for (let i = kMin; i <= k; i++) {
        const logProbI = this.logCombination(K, i) + 
                         this.logCombination(N - K, n - i) - 
                         this.logCombination(N, n)
        cdf += Math.exp(logProbI)
      }
      
      return new CalculationResult(true, pmf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'Fallback calculation failed')
    }
  }

  // Helper mathematical functions
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

  // Standard normal CDF approximation using Abramowitz and Stegun
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

export { NativeBridge, CalculationResult, DISTRIBUTION_TYPES }