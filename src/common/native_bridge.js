import { 
  STAT_SQRT_2PI, 
  ABRAMOWITZ_STEGUN_COEFF, 
  ABRAMOWITZ_STEGUN_CONST, 
  ITERATION_LIMIT, 
  PRECISION_TOLERANCE } from './constants';

let app = null
let NativeModule = null

try {
  if (!NativeModule && app) {
    if (app.invokeNativePlugin) {
      NativeModule = {
        call: (functionName, params) => {
          return app.invokeNativePlugin({
            plugin: 'StatisticalCalculator',
            method: functionName,
            args: params
          })
        },
        type: 'system_plugin'
      }
      console.log('Native module initialization: System plugin mode')
    } else if (app.callNative) {
      NativeModule = {
        call: (functionName, params) => {
          return app.callNative({
            component: 'StatisticalCalculator',
            method: functionName,
            params: params
          })
        },
        type: 'system_native'
      }
      console.log('Native module initialization: System native mode')
    } else if (typeof global !== 'undefined' && global.requireNativePlugin) {
      const nativeModule = global.requireNativePlugin('StatisticalCalculator')
      if (nativeModule && nativeModule.orchestrator_calculate_with_request) {
        NativeModule = {
          call: (functionName, params) => {
            return nativeModule[functionName](params)
          },
          type: 'global_plugin'
        }
        console.log('Native module initialization: Global plugin mode')
      }
    }
  }

  if (!NativeModule) {
    console.log('Native module initialization: JavaScript implementation mode (fallback)')
  }
} catch (error) {
  console.warn('Native module initialization failed:', error)
  NativeModule = null
}

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
  DIST_POISSON: 9,
  DIST_UNIFORM: 10,
  DIST_GAMMA: 11,
  DIST_BETA: 12,
  DIST_WEIBULL: 13,
  DIST_PARETO: 14,
  DIST_RAYLEIGH: 15
}

const MATH_CONSTANTS = {
  SQRT_2PI: STAT_SQRT_2PI,
  INV_SQRT_2PI: 1 / STAT_SQRT_2PI,
  LOG_2PI: Math.log(2 * Math.PI),
  HALF_LOG_2PI: 0.5 * Math.log(2 * Math.PI),
  ABRAMOWITZ_STEGUN_COEFF,
  ABRAMOWITZ_STEGUN_CONST
}

class CalculationResult {
  constructor(success = false, pdfResult = 0, cdfResult = 0, errorMessage = null) {
    this.success = success
    this.pdfResult = pdfResult
    this.pmfResult = pdfResult
    this.cdfResult = cdfResult
    this.errorMessage = errorMessage
  }
}

import LRUCache from './lru_cache'

const memoCache = {
  logFactorial: new LRUCache(1000),
  logGamma: new LRUCache(1000),
  logCombination: new LRUCache(1000)
}

const resultPool = {
  pool: [],
  maxSize: 50,
  
  get() {
    if (this.pool.length > 0) {
      return this.pool.pop()
    }
    return new CalculationResult()
  },
  
  release(result) {
    if (this.pool.length < this.maxSize) {
      result.success = false
      result.pdfResult = 0
      result.pmfResult = 0
      result.cdfResult = 0
      result.errorMessage = null
      this.pool.push(result)
    }
  }
}

class NativeBridge {
  static_originalNativeModule = null

  /**
   * Create an optimized CalculationResult using object pooling
   * @param {boolean} success
   * @param {number} pdfResult
   * @param {number} cdfResult
   * @param {string} errorMessage
   * @returns {CalculationResult}
   */
  static createResult(success = false, pdfResult = 0, cdfResult = 0, errorMessage = null) {
    const result = resultPool.get()
    result.success = success
    result.pdfResult = pdfResult
    result.pmfResult = pdfResult
    result.cdfResult = cdfResult
    result.errorMessage = errorMessage
    return result
  }

  /**
   * Clear memoization caches to free memory
   * @param {string} cacheType - Optional: 'logFactorial', 'logGamma', 'logCombination', or 'all'
   */
  static clearCache(cacheType = 'all') {
    if (cacheType === 'all' || cacheType === 'logFactorial') {
      memoCache.logFactorial.clear()
    }
    if (cacheType === 'all' || cacheType === 'logGamma') {
      memoCache.logGamma.clear()
    }
    if (cacheType === 'all' || cacheType === 'logCombination') {
      memoCache.logCombination.clear()
    }
  }

  /**
   * Get cache statistics for memory monitoring
   * @returns {object} Cache size information
   */
  static getCacheStats() {
    return {
      logFactorial: memoCache.logFactorial.size,
      logGamma: memoCache.logGamma.size,
      logCombination: memoCache.logCombination.size,
      resultPoolSize: resultPool.pool.length,
      totalCacheEntries: memoCache.logFactorial.size + memoCache.logGamma.size + memoCache.logCombination.size
    }
  }

  static binomial(n, p, k) {
    try {
      const logCombo = this.logCombination(n, k)
      const logP = Math.log(p)
      const log1MinusP = Math.log(1 - p)
      const logProb = logCombo + k * logP + (n - k) * log1MinusP
      const pmf = Math.exp(logProb)

      let cdf = 0
      let logCdfTerm = this.logCombination(n, 0) + (n) * log1MinusP
      
      for (let i = 0; i <= k; i++) {
        cdf += Math.exp(logCdfTerm)
        if (i < k) {
          logCdfTerm += Math.log((n - i) / (i + 1)) + logP - log1MinusP
        }
      }

      return this.createResult(true, pmf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static poisson(lambda, k) {
    try {
      const logLambda = Math.log(lambda)
      const logProb = k * logLambda - lambda - this.logFactorial(k)
      const pmf = Math.exp(logProb)

      let cdf = 0
      let logCdfTerm = -lambda
      
      for (let i = 0; i <= k; i++) {
        cdf += Math.exp(logCdfTerm)
        if (i < k) {
          logCdfTerm += logLambda - Math.log(i + 1)
        }
      }

      return this.createResult(true, pmf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static geometric(p, k) {
    try {
      const log1MinusP = Math.log(1 - p)
      const logProb = (k - 1) * log1MinusP + Math.log(p)
      const pmf = Math.exp(logProb)
      const cdf = 1.0 - Math.exp(k * log1MinusP)

      return this.createResult(true, pmf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static negativeBinomial(r, p, k) {
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

      return this.createResult(true, pmf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static normalDistribution(mu, sigma, x) {
    try {
      const z = (x - mu) / sigma
      const pdf = (MATH_CONSTANTS.INV_SQRT_2PI / sigma) * Math.exp(-0.5 * z * z)

      const cdf = this.standardNormalCDF(z)

      return this.createResult(true, pdf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static hypergeometric(N, K, n, k) {
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

      return this.createResult(true, pmf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static exponentialDistribution(lambda, x) {
    try {
      if (lambda <= 0 || x < 0) {
        return this.createResult(false, 0, 0, 'Invalid parameters')
      }

      const pdf = lambda * Math.exp(-lambda * x)
      const cdf = 1 - Math.exp(-lambda * x)

      return this.createResult(true, pdf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static chiSquareDistribution(k, x) {
    try {
      if (k <= 0 || x < 0) {
        return this.createResult(false, 0, 0, 'Invalid parameters')
      }

      const gamma = this.logGamma(k / 2)
      const pdf = Math.pow(x, (k / 2) - 1) * Math.exp(-x / 2) / (Math.pow(2, k / 2) * Math.exp(gamma))

      const cdf = this.incompleteGamma(k / 2, x / 2)

      return this.createResult(true, pdf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static tDistribution(nu, t) {
    try {
      if (nu <= 0) {
        return this.createResult(false, 0, 0, 'Invalid degrees of freedom')
      }

      const pdf = this.tDistributionPDF(t, nu)
      const cdf = this.tDistributionCDF(t, nu)

      return this.createResult(true, pdf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static fDistribution(d1, d2, x) {
    try {
      if (d1 <= 0 || d2 <= 0 || x < 0) {
        return this.createResult(false, 0, 0, 'Invalid parameters')
      }

      const pdf = this.fDistributionPDF(x, d1, d2)
      const cdf = this.fDistributionCDF(x, d1, d2)

      return this.createResult(true, pdf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static uniformDistribution(a, b, x) {
    try {
      if (a >= b) {
        return this.createResult(false, 0, 0, 'Invalid parameters: a must be less than b')
      }

      let pdf = 0
      let cdf = 0

      if (x >= a && x <= b) {
        pdf = 1 / (b - a)
        cdf = (x - a) / (b - a)
      } else if (x < a) {
        pdf = 0
        cdf = 0
      } else {
        pdf = 0
        cdf = 1
      }

      return this.createResult(true, pdf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static gammaDistribution(k, theta, x) {
    try {
      if (k <= 0 || theta <= 0 || x < 0) {
        return this.createResult(false, 0, 0, 'Invalid parameters')
      }

      const gamma = this.logGamma(k)
      const pdf = Math.pow(x, k - 1) * Math.exp(-x / theta) / (Math.pow(theta, k) * Math.exp(gamma))

      const cdf = this.incompleteGamma(k, x / theta)

      return this.createResult(true, pdf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static betaDistribution(alpha, beta, x) {
    try {
      if (alpha <= 0 || beta <= 0 || x < 0 || x > 1) {
        return this.createResult(false, 0, 0, 'Invalid parameters')
      }

      const logBeta = this.logBeta(alpha, beta)
      const pdf = Math.pow(x, alpha - 1) * Math.pow(1 - x, beta - 1) / Math.exp(logBeta)

      const cdf = this.incompleteBeta(x, alpha, beta)

      return this.createResult(true, pdf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static weibullDistribution(k, lambda, x) {
    try {
      if (k <= 0 || lambda <= 0 || x < 0) {
        return this.createResult(false, 0, 0, 'Invalid parameters')
      }

      const pdf = (k / lambda) * Math.pow(x / lambda, k - 1) * Math.exp(-Math.pow(x / lambda, k))
      const cdf = 1 - Math.exp(-Math.pow(x / lambda, k))

      return this.createResult(true, pdf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static paretoDistribution(alpha, xm, x) {
    try {
      if (alpha <= 0 || xm <= 0 || x < xm) {
        return this.createResult(false, 0, 0, 'Invalid parameters: α > 0, xm > 0, x ≥ xm')
      }

      const pdf = (alpha * Math.pow(xm, alpha)) / Math.pow(x, alpha + 1)
      const cdf = 1 - Math.pow(xm / x, alpha)

      return this.createResult(true, pdf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static rayleighDistribution(sigma, x) {
    try {
      if (sigma <= 0 || x < 0) {
        return this.createResult(false, 0, 0, 'Invalid parameters')
      }

      const pdf = (x / (sigma * sigma)) * Math.exp(-(x * x) / (2 * sigma * sigma))
      const cdf = 1 - Math.exp(-(x * x) / (2 * sigma * sigma))

      return this.createResult(true, pmf, cdf)
    } catch (error) {
      return this.createResult(false, 0, 0, 'JavaScript calculation failed')
    }
  }

  static logCombination(n, k) {
    if (k > n || k < 0) return -Infinity
    if (k === 0 || k === n) return 0

    const key = `${n},${k}`
    const cachedResult = memoCache.logCombination.get(key)
    if (cachedResult !== null) {
      return cachedResult
    }

    const result = this.logFactorial(n) - this.logFactorial(k) - this.logFactorial(n - k)
    memoCache.logCombination.set(key, result)

    return result
  }

  static logFactorial(n) {
    if (n <= 1) return 0

    const cachedResult = memoCache.logFactorial.get(n)
    if (cachedResult !== null) {
      return cachedResult
    }

    let result = 0
    for (let i = 2; i <= n; i++) {
      result += Math.log(i)
    }

    memoCache.logFactorial.set(n, result)
    return result
  }

  static logGamma(z) {
    const key = z.toString()
    const cachedResult = memoCache.logGamma.get(key)
    if (cachedResult !== null) {
      return cachedResult
    }

    const originalZ = z
    if (z < 1) z += 1
    const result = (z - 0.5) * Math.log(z) - z + MATH_CONSTANTS.HALF_LOG_2PI + 1 / (12 * z) - 1 / (360 * z * z * z)

    memoCache.logGamma.set(originalZ.toString(), result)
    return result
  }

  static logBeta(a, b) {
    return this.logGamma(a) + this.logGamma(b) - this.logGamma(a + b)
  }

  static incompleteGamma(s, x) {
    if (x <= 0) return 0
    if (s <= 0) return 1

    let sum = 0
    let term = Math.exp(-x + s * Math.log(x) - this.logGamma(s))
    for (let k = 0; k < ITERATION_LIMIT; k++) {
      sum += term
      term *= x / (s + k + 1)
      if (Math.abs(term) < PRECISION_TOLERANCE) break
    }
    return sum
  }

  static incompleteBeta(x, a, b) {
    if (x <= 0) return 0
    if (x >= 1) return 1

    let sum = 0

    for (let k = 0; k < ITERATION_LIMIT; k++) {
      const logTerm = this.logGamma(a + k) - this.logGamma(a) + this.logGamma(b) - this.logGamma(b + k) +
        this.logGamma(a + b) - this.logGamma(a + b + k) + Math.log(x) * k - Math.log(k + 1)
      sum += Math.exp(logTerm)
      if (Math.abs(Math.exp(logTerm)) < PRECISION_TOLERANCE) break
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

  static standardNormalCDF(z) {
    if (z < 0) return 1 - this.standardNormalCDF(-z)

    const t = 1 / (1 + MATH_CONSTANTS.ABRAMOWITZ_STEGUN_CONST * z)
    const coeffs = MATH_CONSTANTS.ABRAMOWITZ_STEGUN_COEFF

    let tPower = t
    let polynomial = coeffs[0] * tPower
    
    for (let i = 1; i < coeffs.length; i++) {
      tPower *= t
      polynomial += coeffs[i] * tPower
    }

    const pdf = MATH_CONSTANTS.INV_SQRT_2PI * Math.exp(-0.5 * z * z)
    return 1 - pdf * polynomial
  }
}

export { NativeBridge, CalculationResult, DISTRIBUTION_TYPES }