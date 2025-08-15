import app from '@system.app'

let NativeModule = null

try {
  if (app) {
    if (app.invokeNativePlugin) {
      NativeModule = {
        call: (functionName, params) => {
          return app.invokeNativePlugin({
            plugin: 'StatisticalCalculator',
            method: functionName,
            args: params
          })
        }
      }
    } else if (app.callNative) {
      NativeModule = {
        call: (functionName, params) => {
          return app.callNative({
            component: 'StatisticalCalculator',
            method: functionName,
            params: params
          })
        }
      }
    } else if (typeof global !== 'undefined' && global.requireNativePlugin) {
      const nativeModule = global.requireNativePlugin('StatisticalCalculator')
      if (nativeModule && nativeModule.orchestrator_calculate_with_request) {
        NativeModule = {
          call: (functionName, params) => {
            return nativeModule[functionName](params)
          }
        }
      }
    }
  }

  // If no native module found, will use JavaScript implementation
  if (!NativeModule) {
    console.log('Native module initialization: JavaScript implementation mode')
  } else {
    console.log('Native module initialization: Native C engine mode')
  }
} catch (error) {
  console.warn('Native module initialization failed:', error)
  NativeModule = null
}

/**
 * Distribution type constants matching C enum
 * Note: Uniform, Gamma, and Beta distributions are only available in JavaScript fallback
 */
const DISTRIBUTION_TYPES = {
  // C enum values (0-9)
  DIST_NORMAL: 0,
  DIST_EXPONENTIAL: 1,
  DIST_CHI_SQUARE: 2,
  DIST_T_DISTRIBUTION: 3,
  DIST_F_DISTRIBUTION: 4,
  DIST_GEOMETRIC: 5,
  DIST_HYPERGEOMETRIC: 6,
  DIST_BINOMIAL: 7,
  DIST_NEGATIVE_BINOMIAL: 8,
  DIST_POISSON: 9,
  
  // JavaScript-only values (10+)
  DIST_UNIFORM: 10,
  DIST_GAMMA: 11,
  DIST_BETA: 12
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
   * Check if native module is available
   * @returns {boolean}
   */
  static isNativeModuleAvailable() {
    return !!(NativeModule && typeof NativeModule.call === 'function')
  }

  /**
   * Force use of JavaScript implementation for testing purposes
   * @param {boolean} forceJavaScript - Whether to force JavaScript mode
   */
  static setForceJavaScript(forceJavaScript) {
    if (forceJavaScript) {
      this._originalNativeModule = NativeModule
      NativeModule = null
    } else if (this._originalNativeModule) {
      NativeModule = this._originalNativeModule
      this._originalNativeModule = null
    }
  }

  /**
   * Get current calculation mode
   * @returns {string} 'native' or 'javascript'
   */
  static getCalculationMode() {
    if (NativeModule && typeof NativeModule.call === 'function') {
      return 'native'
    } else {
      return 'javascript'
    }
  }

  /**
   * Calculate binomial distribution
   * @param {number} n - number of trials
   * @param {number} p - probability of success
   * @param {number} k - number of successes
   * @returns {CalculationResult}
   */
  static calculateBinomial(n, p, k) {
    try {
      // Try native calculation first
      if (NativeModule && typeof NativeModule.call === 'function') {
        const params = [n, p]
        const request = {
          distribution: DISTRIBUTION_TYPES.DIST_BINOMIAL,
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
      } else {
        // Use JavaScript implementation
        console.log('Using JavaScript implementation for binomial calculation')
        return this.jsBinomial(n, p, k)
      }
    } catch (error) {
      console.error('Native binomial calculation failed, using JavaScript implementation:', error)
      return this.jsBinomial(n, p, k)
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
      // Try native calculation first
      if (NativeModule && typeof NativeModule.call === 'function') {
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
      } else {
        // Use JavaScript implementation
        console.log('Using JavaScript implementation for Poisson calculation')
        return this.jsPoisson(lambda, k)
      }
    } catch (error) {
      console.error('Native Poisson calculation failed, using JavaScript implementation:', error)
      return this.jsPoisson(lambda, k)
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
      // Try native calculation first
      if (NativeModule && typeof NativeModule.call === 'function') {
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
      } else {
        // Use JavaScript implementation
        console.log('Using JavaScript implementation for geometric calculation')
        return this.jsGeometric(p, k)
      }
    } catch (error) {
      console.error('Native geometric calculation failed, using JavaScript implementation:', error)
      return this.jsGeometric(p, k)
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
      // Try native calculation first
      if (NativeModule && typeof NativeModule.call === 'function') {
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
      } else {
        // Use JavaScript implementation
        console.log('Using JavaScript implementation for negative binomial calculation')
        return this.jsNegativeBinomial(r, p, k)
      }
    } catch (error) {
      console.error('Native negative binomial calculation failed, using JavaScript implementation:', error)
      return this.jsNegativeBinomial(r, p, k)
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
      // Try native calculation first
      if (NativeModule && typeof NativeModule.call === 'function') {
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
      } else {
        // Use JavaScript implementation
        console.log('Using JavaScript implementation for normal distribution calculation')
        return this.jsNormalDistribution(mu, sigma, x)
      }
    } catch (error) {
      console.error('Native normal distribution calculation failed, using JavaScript implementation:', error)
      return this.jsNormalDistribution(mu, sigma, x)
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
      // Try native calculation first
      if (NativeModule && typeof NativeModule.call === 'function') {
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
      } else {
        // Use JavaScript implementation
        console.log('Using JavaScript implementation for hypergeometric calculation')
        return this.jsHypergeometric(N, K, n, k)
      }
    } catch (error) {
      console.error('Native hypergeometric calculation failed, using JavaScript implementation:', error)
      return this.jsHypergeometric(N, K, n, k)
    }
  }

  /**
   * Calculate exponential distribution
   * @param {number} lambda - rate parameter
   * @param {number} x - input value
   * @returns {CalculationResult}
   */
  static calculateExponentialDistribution(lambda, x) {
    try {
      // Try native calculation first
      if (NativeModule && typeof NativeModule.call === 'function') {
        const params = [lambda]
        const request = {
          distribution: DISTRIBUTION_TYPES.DIST_EXPONENTIAL,
          parameters: params,
          param_count: 1,
          input_value: x
        }

        const result = NativeModule.call('orchestrator_calculate_with_request', request)
        return new CalculationResult(
          result.success === 1,
          result.pdf_result,
          result.cdf_result,
          result.error_message
        )
      } else {
        // Use JavaScript implementation
        console.log('Using JavaScript implementation for exponential distribution calculation')
        return this.jsExponentialDistribution(lambda, x)
      }
    } catch (error) {
      console.error('Native exponential distribution calculation failed, using JavaScript implementation:', error)
      return this.jsExponentialDistribution(lambda, x)
    }
  }

  /**
   * Calculate chi-square distribution
   * @param {number} k - degrees of freedom
   * @param {number} x - input value
   * @returns {CalculationResult}
   */
  static calculateChiSquareDistribution(k, x) {
    try {
      // Try native calculation first
      if (NativeModule && typeof NativeModule.call === 'function') {
        const params = [k]
        const request = {
          distribution: DISTRIBUTION_TYPES.DIST_CHI_SQUARE,
          parameters: params,
          param_count: 1,
          input_value: x
        }

        const result = NativeModule.call('orchestrator_calculate_with_request', request)
        return new CalculationResult(
          result.success === 1,
          result.pdf_result,
          result.cdf_result,
          result.error_message
        )
      } else {
        // Use JavaScript implementation
        console.log('Using JavaScript implementation for chi-square distribution calculation')
        return this.jsChiSquareDistribution(k, x)
      }
    } catch (error) {
      console.error('Native chi-square distribution calculation failed, using JavaScript implementation:', error)
      return this.jsChiSquareDistribution(k, x)
    }
  }

  /**
   * Calculate t-distribution
   * @param {number} nu - degrees of freedom
   * @param {number} t - input value
   * @returns {CalculationResult}
   */
  static calculateTDistribution(nu, t) {
    try {
      // Try native calculation first
      if (NativeModule && typeof NativeModule.call === 'function') {
        const params = [nu]
        const request = {
          distribution: DISTRIBUTION_TYPES.DIST_T_DISTRIBUTION,
          parameters: params,
          param_count: 1,
          input_value: t
        }

        const result = NativeModule.call('orchestrator_calculate_with_request', request)
        return new CalculationResult(
          result.success === 1,
          result.pdf_result,
          result.cdf_result,
          result.error_message
        )
      } else {
        // Use JavaScript implementation
        console.log('Using JavaScript implementation for t-distribution calculation')
        return this.jsTDistribution(nu, t)
      }
    } catch (error) {
      console.error('Native t-distribution calculation failed, using JavaScript implementation:', error)
      return this.jsTDistribution(nu, t)
    }
  }

  /**
   * Calculate F-distribution
   * @param {number} d1 - numerator degrees of freedom
   * @param {number} d2 - denominator degrees of freedom
   * @param {number} x - input value
   * @returns {CalculationResult}
   */
  static calculateFDistribution(d1, d2, x) {
    try {
      // Try native calculation first
      if (NativeModule && typeof NativeModule.call === 'function') {
        const params = [d1, d2]
        const request = {
          distribution: DISTRIBUTION_TYPES.DIST_F_DISTRIBUTION,
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
      } else {
        // Use JavaScript implementation
        console.log('Using JavaScript implementation for F-distribution calculation')
        return this.jsFDistribution(d1, d2, x)
      }
    } catch (error) {
      console.error('Native F-distribution calculation failed, using JavaScript implementation:', error)
      return this.jsFDistribution(d1, d2, x)
    }
  }

  /**
   * Calculate uniform distribution
   * @param {number} a - lower bound
   * @param {number} b - upper bound
   * @param {number} x - input value
   * @returns {CalculationResult}
   */
  static calculateUniformDistribution(a, b, x) {
    // Uniform distribution implemented in JavaScript
    console.log('Using JavaScript implementation for uniform distribution calculation')
    return this.jsUniformDistribution(a, b, x)
  }

  /**
   * Calculate gamma distribution
   * @param {number} k - shape parameter
   * @param {number} theta - scale parameter
   * @param {number} x - input value
   * @returns {CalculationResult}
   */
  static calculateGammaDistribution(k, theta, x) {
    // Gamma distribution implemented in JavaScript
    console.log('Using JavaScript implementation for gamma distribution calculation')
    return this.jsGammaDistribution(k, theta, x)
  }

  /**
   * Calculate beta distribution
   * @param {number} alpha - first shape parameter
   * @param {number} beta - second shape parameter
   * @param {number} x - input value
   * @returns {CalculationResult}
   */
  static calculateBetaDistribution(alpha, beta, x) {
    // Beta distribution implemented in JavaScript
    console.log('Using JavaScript implementation for beta distribution calculation')
    return this.jsBetaDistribution(alpha, beta, x)
  }

  /**
   * Calculate Weibull distribution
   * @param {number} k - shape parameter
   * @param {number} lambda - scale parameter
   * @param {number} x - input value
   * @returns {CalculationResult}
   */
  static calculateWeibullDistribution(k, lambda, x) {
    // Weibull distribution implemented in JavaScript
    console.log('Using JavaScript implementation for Weibull distribution calculation')
    return this.jsWeibullDistribution(k, lambda, x)
  }

  /**
   * Calculate Student's t distribution
   * @param {number} df - degrees of freedom
   * @param {number} x - input value
   * @returns {CalculationResult}
   */
  static calculateStudentTDistribution(df, x) {
    // Use the existing t-distribution method
    return this.calculateTDistribution(df, x)
  }

  /**
   * Calculate Pareto distribution
   * @param {number} alpha - shape parameter
   * @param {number} xm - scale parameter
   * @param {number} x - input value
   * @returns {CalculationResult}
   */
  static calculateParetoDistribution(alpha, xm, x) {
    // Pareto distribution implemented in JavaScript
    console.log('Using JavaScript implementation for Pareto distribution calculation')
    return this.jsParetoDistribution(alpha, xm, x)
  }

  /**
   * Calculate Rayleigh distribution
   * @param {number} sigma - scale parameter
   * @param {number} x - input value
   * @returns {CalculationResult}
   */
  static calculateRayleighDistribution(sigma, x) {
    // Rayleigh distribution implemented in JavaScript
    console.log('Using JavaScript implementation for Rayleigh distribution calculation')
    return this.jsRayleighDistribution(sigma, x)
  }

  // JavaScript implementations
  // Used when native C module is not available or for distributions not implemented in C

  static jsBinomial(n, p, k) {
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
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static jsPoisson(lambda, k) {
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
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static jsGeometric(p, k) {
    try {
      const logProb = (k - 1) * Math.log(1 - p) + Math.log(p)
      const pmf = Math.exp(logProb)
      const cdf = 1.0 - Math.exp(k * Math.log(1 - p))
      
      return new CalculationResult(true, pmf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static jsNegativeBinomial(r, p, k) {
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
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static jsNormalDistribution(mu, sigma, x) {
    try {
      // Standard normal PDF calculation
      const z = (x - mu) / sigma
      const pdf = (1 / (sigma * Math.sqrt(2 * Math.PI))) * Math.exp(-0.5 * z * z)
      
      // Approximate CDF using Abramowitz and Stegun approximation
      const cdf = this.standardNormalCDF(z)
      
      return new CalculationResult(true, pdf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static jsHypergeometric(N, K, n, k) {
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
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static jsExponentialDistribution(lambda, x) {
    try {
      if (lambda <= 0 || x < 0) {
        return new CalculationResult(false, 0, 0, 'Invalid parameters')
      }
      
      const pdf = lambda * Math.exp(-lambda * x)
      const cdf = 1 - Math.exp(-lambda * x)
      
      return new CalculationResult(true, pdf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static jsChiSquareDistribution(k, x) {
    try {
      if (k <= 0 || x < 0) {
        return new CalculationResult(false, 0, 0, 'Invalid parameters')
      }
      
      // Using chi-square approximation
      const gamma = this.logGamma(k / 2)
      const pdf = Math.pow(x, (k / 2) - 1) * Math.exp(-x / 2) / (Math.pow(2, k / 2) * Math.exp(gamma))
      
      // Approximate CDF using incomplete gamma function approximation
      // This is a simplified version - in practice, you'd use a more sophisticated method
      const cdf = this.incompleteGamma(k / 2, x / 2)
      
      return new CalculationResult(true, pdf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static jsTDistribution(nu, t) {
    try {
      if (nu <= 0) {
        return new CalculationResult(false, 0, 0, 'Invalid degrees of freedom')
      }
      
      const pdf = this.tDistributionPDF(t, nu)
      const cdf = this.tDistributionCDF(t, nu)
      
      return new CalculationResult(true, pdf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static jsFDistribution(d1, d2, x) {
    try {
      if (d1 <= 0 || d2 <= 0 || x < 0) {
        return new CalculationResult(false, 0, 0, 'Invalid parameters')
      }
      
      const pdf = this.fDistributionPDF(x, d1, d2)
      const cdf = this.fDistributionCDF(x, d1, d2)
      
      return new CalculationResult(true, pdf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static jsUniformDistribution(a, b, x) {
    try {
      if (a >= b) {
        return new CalculationResult(false, 0, 0, 'Invalid parameters: a must be less than b')
      }
      
      let pdf = 0
      let cdf = 0
      
      if (x >= a && x <= b) {
        pdf = 1 / (b - a)
        cdf = (x - a) / (b - a)
      } else if (x < a) {
        pdf = 0
        cdf = 0
      } else { // x > b
        pdf = 0
        cdf = 1
      }
      
      return new CalculationResult(true, pdf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static jsGammaDistribution(k, theta, x) {
    try {
      if (k <= 0 || theta <= 0 || x < 0) {
        return new CalculationResult(false, 0, 0, 'Invalid parameters')
      }
      
      const gamma = this.logGamma(k)
      const pdf = Math.pow(x, k - 1) * Math.exp(-x / theta) / (Math.pow(theta, k) * Math.exp(gamma))
      
      // Approximate CDF using incomplete gamma function
      const cdf = this.incompleteGamma(k, x / theta)
      
      return new CalculationResult(true, pdf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static jsBetaDistribution(alpha, beta, x) {
    try {
      if (alpha <= 0 || beta <= 0 || x < 0 || x > 1) {
        return new CalculationResult(false, 0, 0, 'Invalid parameters')
      }
      
      const logBeta = this.logBeta(alpha, beta)
      const pdf = Math.pow(x, alpha - 1) * Math.pow(1 - x, beta - 1) / Math.exp(logBeta)
      
      // Approximate CDF using incomplete beta function
      const cdf = this.incompleteBeta(x, alpha, beta)
      
      return new CalculationResult(true, pdf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static jsWeibullDistribution(k, lambda, x) {
    try {
      if (k <= 0 || lambda <= 0 || x < 0) {
        return new CalculationResult(false, 0, 0, 'Invalid parameters')
      }
      
      const pdf = (k / lambda) * Math.pow(x / lambda, k - 1) * Math.exp(-Math.pow(x / lambda, k))
      const cdf = 1 - Math.exp(-Math.pow(x / lambda, k))
      
      return new CalculationResult(true, pdf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static jsParetoDistribution(alpha, xm, x) {
    try {
      if (alpha <= 0 || xm <= 0 || x < xm) {
        return new CalculationResult(false, 0, 0, 'Invalid parameters: α > 0, xm > 0, x ≥ xm')
      }
      
      const pdf = (alpha * Math.pow(xm, alpha)) / Math.pow(x, alpha + 1)
      const cdf = 1 - Math.pow(xm / x, alpha)
      
      return new CalculationResult(true, pdf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static jsRayleighDistribution(sigma, x) {
    try {
      if (sigma <= 0 || x < 0) {
        return new CalculationResult(false, 0, 0, 'Invalid parameters')
      }
      
      const pdf = (x / (sigma * sigma)) * Math.exp(-(x * x) / (2 * sigma * sigma))
      const cdf = 1 - Math.exp(-(x * x) / (2 * sigma * sigma))
      
      return new CalculationResult(true, pdf, cdf)
    } catch (error) {
      return new CalculationResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  // Helper mathematical functions for JavaScript calculations
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

  static logGamma(z) {
    // Stirling's approximation for log gamma
    if (z < 1) z += 1
    return (z - 0.5) * Math.log(z) - z + 0.5 * Math.log(2 * Math.PI) + 1/(12 * z) - 1/(360 * z * z * z)
  }

  static logBeta(a, b) {
    return this.logGamma(a) + this.logGamma(b) - this.logGamma(a + b)
  }

  static incompleteGamma(s, x) {
    // Simplified incomplete gamma function approximation
    if (x <= 0) return 0
    if (s <= 0) return 1
    
    let sum = 0
    let term = Math.exp(-x + s * Math.log(x) - this.logGamma(s))
    for (let k = 0; k < 100; k++) {
      sum += term
      term *= x / (s + k + 1)
      if (Math.abs(term) < 1e-15) break
    }
    return sum
  }

  static incompleteBeta(x, a, b) {
    // Simplified incomplete beta function approximation
    if (x <= 0) return 0
    if (x >= 1) return 1
    
    let sum = 0
    
    for (let k = 0; k < 100; k++) {
      const logTerm = this.logGamma(a + k) - this.logGamma(a) + this.logGamma(b) - this.logGamma(b + k) + 
                     this.logGamma(a + b) - this.logGamma(a + b + k) + Math.log(x) * k - Math.log(k + 1)
      sum += Math.exp(logTerm)
      if (Math.abs(Math.exp(logTerm)) < 1e-15) break
    }
    
    return Math.exp(this.logGamma(a + b) - this.logGamma(a) - this.logGamma(b) + a * Math.log(x)) * sum
  }

  static tDistributionPDF(t, nu) {
    const pdf = Math.exp(this.logGamma((nu + 1) / 2) - 0.5 * Math.log(Math.PI * nu) - this.logGamma(nu / 2) - 
                        (nu + 1) / 2 * Math.log(1 + t * t / nu))
    return pdf
  }

  static tDistributionCDF(t, nu) {
    if (t === 0) return 0.5
    
    const x = nu / (t * t + nu)
    const sign = t > 0 ? 1 : -1
    
    // Use incomplete beta function approximation
    const cdf = 0.5 + 0.5 * sign * this.incompleteBeta(x, nu / 2, 0.5)
    return Math.max(0, Math.min(1, cdf))
  }

  static fDistributionPDF(x, d1, d2) {
    if (x <= 0) return 0
    
    const logBeta = this.logBeta(d1 / 2, d2 / 2)
    const pdf = Math.exp((d1 / 2) * Math.log(d1 / d2) + (d1 / 2 - 1) * Math.log(x) - 
                        ((d1 + d2) / 2) * Math.log(1 + (d1 / d2) * x) - logBeta)
    return pdf
  }

  static fDistributionCDF(x, d1, d2) {
    if (x <= 0) return 0
    
    const z = (d1 * x) / (d1 * x + d2)
    return this.incompleteBeta(z, d1 / 2, d2 / 2)
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