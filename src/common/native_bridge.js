import jstat from 'jstat';
import { 
  STAT_SQRT_2PI, 
  ABRAMOWITZ_STEGUN_COEFF, 
  ABRAMOWITZ_STEGUN_CONST } from './constants.js';

// Directly use the JS calculation to prevent error output in console and cause delay
console.log('Native module initialization: JavaScript implementation mode (fallback)');

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
  constructor(success = false, pdfResult = 0, cdfResult = 0, errorMessage = null, chartData = null) {
    this.success = success
    this.pdfResult = pdfResult
    this.pmfResult = pdfResult
    this.cdfResult = cdfResult
    this.errorMessage = errorMessage
    this.chartData = chartData
  }
}

import LRUCache from './lru_cache.js'

const memoCache = {
  logFactorial: new LRUCache(500), // Reduced from 1000 for memory efficiency
  logGamma: new LRUCache(500),     // Reduced from 1000 for memory efficiency  
  logCombination: new LRUCache(500) // Reduced from 1000 for memory efficiency
}

const resultPool = {
  pool: [],
  maxSize: 25, // Reduced from 50 for memory efficiency 
  // "https://www.makeuseof.com/improve-performance-free-up-ram-on-linux/"
  
  get() {
    if (this.pool.length > 0) {
      return this.pool.pop()
    }
    return new CalculationResult()
  },
  
  release(result) {
    if (this.pool.length < this.maxSize) {
      // Clear all references
      result.success = false
      result.pdfResult = 0
      result.pmfResult = 0
      result.cdfResult = 0
      result.errorMessage = null
      result.chartData = null // Clear chart data to free memory
      this.pool.push(result)
    }
  },
  
  // Force cleanup of result pool
  cleanup() {
    this.pool.length = 0
  }
}

// Add periodic cleanup for caches
setInterval(() => {
  // Clear caches if they get too large
  if (memoCache.logFactorial.size > 300) {
    memoCache.logFactorial.clear()
  }
  if (memoCache.logGamma.size > 300) {
    memoCache.logGamma.clear()
  }
  if (memoCache.logCombination.size > 300) {
    memoCache.logCombination.clear()
  }
  
  // Cleanup result pool periodically
  if (resultPool.pool.length > 15) {
    resultPool.cleanup()
  }
}, 30000) // Every 30 seconds

class NativeBridge {

  static createResult(success = false, pdfResult = 0, cdfResult = 0, errorMessage = null) {
    const result = resultPool.get()
    result.success = success
    result.pdfResult = pdfResult
    result.pmfResult = pdfResult
    result.cdfResult = cdfResult
    result.errorMessage = errorMessage
    return result
  }

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

    const chartData = this.generateBinomialChartData(n, p);
    return this.createResult(true, pmf, cdf, null, chartData);
  }

  static generateBinomialChartData(n, p) {
    const labels = [];
    const data = [];
    for (let k = 0; k <= n; k++) {
      labels.push(k);
      const logCombo = this.logCombination(n, k);
      const logP = Math.log(p);
      const log1MinusP = Math.log(1 - p);
      const logProb = logCombo + k * logP + (n - k) * log1MinusP;
      data.push(Math.exp(logProb));
    }

    return {
      labels,
      datasets: [
        {
          label: "PMF",
          data,
          backgroundColor: "#00ff88",
        },
      ],
    };
  }

  static poisson(lambda, k) {
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

    const chartData = this.generatePoissonChartData(lambda);
    return this.createResult(true, pmf, cdf, null, chartData);
  }

  static generatePoissonChartData(lambda) {
    const labels = [];
    const data = [];
    const maxK = Math.max(20, Math.ceil(lambda + 5 * Math.sqrt(lambda)));

    for (let k = 0; k <= maxK; k++) {
      labels.push(k);
      const logProb = k * Math.log(lambda) - lambda - this.logFactorial(k);
      data.push(Math.exp(logProb));
    }

    return {
      labels,
      datasets: [
        {
          label: "PMF",
          data,
          backgroundColor: "#00ff88",
        },
      ],
    };
  }

  static geometric(p, k) {
    const log1MinusP = Math.log(1 - p)
    const logProb = (k - 1) * log1MinusP + Math.log(p)
    const pmf = Math.exp(logProb)
    const cdf = 1.0 - Math.exp(k * log1MinusP)

    return this.createResult(true, pmf, cdf)
  }

  static negativeBinomial(r, p, k) {
    const pmf = jstat.negbin.pdf(k, r, p);
    const cdf = jstat.negbin.cdf(k, r, p);
    return this.createResult(true, pmf, cdf);
  }

  static hypergeometric(N, K, n, k) {
    const pmf = jstat.hypgeom.pdf(k, N, K, n);
    const cdf = jstat.hypgeom.cdf(k, N, K, n);
    return this.createResult(true, pmf, cdf);
  }

  static chiSquareDistribution(k, x) {
    if (k <= 0 || x < 0) {
      return this.createResult(false, 0, 0, 'Invalid parameters');
    }
    const pdf = jstat.chisquare.pdf(x, k);
    const cdf = jstat.chisquare.cdf(x, k);
    return this.createResult(true, pdf, cdf);
  }

  static tDistribution(nu, t) {
    if (nu <= 0) {
      return this.createResult(false, 0, 0, 'Invalid degrees of freedom');
    }
    const pdf = jstat.studentt.pdf(t, nu);
    const cdf = jstat.studentt.cdf(t, nu);
    return this.createResult(true, pdf, cdf);
  }

  static fDistribution(d1, d2, x) {
    if (d1 <= 0 || d2 <= 0 || x < 0) {
      return this.createResult(false, 0, 0, 'Invalid parameters');
    }
    const pdf = jstat.centralF.pdf(x, d1, d2);
    const cdf = jstat.centralF.cdf(x, d1, d2);
    return this.createResult(true, pdf, cdf);
  }

  static betaDistribution(alpha, beta, x) {
    if (alpha <= 0 || beta <= 0 || x < 0 || x > 1) {
      return this.createResult(false, 0, 0, 'Invalid parameters');
    }
    const pdf = jstat.beta.pdf(x, alpha, beta);
    const cdf = jstat.beta.cdf(x, alpha, beta);
    return this.createResult(true, pdf, cdf);
  }

  static normalDistribution(mu, sigma, x) {
    const z = (x - mu) / sigma
    const pdf = (MATH_CONSTANTS.INV_SQRT_2PI / sigma) * Math.exp(-0.5 * z * z)
    const cdf = this.standardNormalCDF(z)
    const chartData = this.generateNormalChartData(mu, sigma)
    return this.createResult(true, pdf, cdf, null, chartData)
  }

  static generateNormalChartData(mu, sigma) {
    const dataPoints = 100;
    const range = 8 * sigma;
    const step = range / dataPoints;
    const startX = mu - 4 * sigma;
    
    const labels = [];
    const data = [];
    
    for (let i = 0; i <= dataPoints; i++) {
      const x = startX + i * step;
      const z = (x - mu) / sigma;
      const pdf = (MATH_CONSTANTS.INV_SQRT_2PI / sigma) * Math.exp(-0.5 * z * z);
      labels.push(x.toFixed(2));
      data.push(pdf);
    }
    
    return {
      labels,
      datasets: [
        {
          label: "PDF",
          data,
          borderColor: "#00ff88",
          fill: false,
        },
      ],
    };
  }

  static exponentialDistribution(lambda, x) {
    if (lambda <= 0 || x < 0) {
      return this.createResult(false, 0, 0, 'Invalid parameters')
    }
    const pdf = lambda * Math.exp(-lambda * x)
    const cdf = 1 - Math.exp(-lambda * x)
    return this.createResult(true, pdf, cdf)
  }

  static uniformDistribution(a, b, x) {
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
  }

  static gammaDistribution(k, theta, x) {
    if (k <= 0 || theta <= 0 || x < 0) {
      return this.createResult(false, 0, 0, 'Invalid parameters')
    }
    const pdf = jstat.gamma.pdf(x, k, theta);
    const cdf = jstat.gamma.cdf(x, k, theta);
    return this.createResult(true, pdf, cdf)
  }

  static weibullDistribution(k, lambda, x) {
    if (k <= 0 || lambda <= 0 || x < 0) {
      return this.createResult(false, 0, 0, 'Invalid parameters')
    }
    const pdf = (k / lambda) * Math.pow(x / lambda, k - 1) * Math.exp(-Math.pow(x / lambda, k))
    const cdf = 1 - Math.exp(-Math.pow(x / lambda, k))
    return this.createResult(true, pdf, cdf)
  }

  static paretoDistribution(alpha, xm, x) {
    if (alpha <= 0 || xm <= 0 || x < xm) {
      return this.createResult(false, 0, 0, 'Invalid parameters: α > 0, xm > 0, x ≥ xm')
    }
    const pdf = (alpha * Math.pow(xm, alpha)) / Math.pow(x, alpha + 1)
    const cdf = 1 - Math.pow(xm / x, alpha)
    return this.createResult(true, pdf, cdf)
  }

  static rayleighDistribution(sigma, x) {
    if (sigma <= 0 || x < 0) {
      return this.createResult(false, 0, 0, 'Invalid parameters')
    }
    const pdf = (x / (sigma * sigma)) * Math.exp(-(x * x) / (2 * sigma * sigma))
    const cdf = 1 - Math.exp(-(x * x) / (2 * sigma * sigma))
    return this.createResult(true, pdf, cdf)
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
