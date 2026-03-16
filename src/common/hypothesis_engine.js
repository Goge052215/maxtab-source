import jstat from 'jstat';

class HypothesisEngine {
  /**
   * Validate statistics parameters
   * @param {object} params
   * @returns {object|null} Error object or null if valid
   */
  static _validateParams(params) {
    if (params.n !== undefined && params.n <= 0) return { error: 'invalid_n', message: 'Sample size must be > 0' };
    if (params.s !== undefined && params.s < 0) return { error: 'invalid_s', message: 'Standard deviation must be >= 0' };
    if (params.sigma !== undefined && params.sigma <= 0) return { error: 'invalid_sigma', message: 'Sigma must be > 0' };
    return null;
  }

  /**
   * Calculate summary statistics from an array of data.
   * @param {number[]} data - Array of numerical values.
   * @returns {object} { n, mean, s, variance }
   */
  static getStats(data) {
    if (!data || !Array.isArray(data) || data.length === 0) {
      return { n: 0, mean: 0, s: 0, variance: 0 };
    }
    const validData = data.filter(Number.isFinite);
    if (validData.length === 0) return { n: 0, mean: 0, s: 0, variance: 0 };

    const n = validData.length;
    const mean = jstat.mean(validData);
    const s = jstat.stdev(validData, true); // true for sample stdev (n-1)
    const variance = jstat.variance(validData, true); // true for sample variance (n-1)
    return { n, mean, s, variance };
  }

  /**
   * One-Sample Z-Test
   * H0: mu = mu0
   * @param {object} params
   * @param {number} params.mean - Sample mean (x_bar)
   * @param {number} params.n - Sample size
   * @param {number} params.sigma - Population standard deviation
   * @param {number} params.mu0 - Hypothesized population mean
   * @param {number} [params.alpha=0.05] - Significance level
   * @returns {object} { z, pValueTwoTail, pValueLeft, pValueRight, lowerCI, upperCI }
   */
  static zTestOneSample({ mean, n, sigma, mu0, alpha = 0.05 }) {
    const error = this._validateParams({ n, sigma });
    if (error) return error;

    const z = (mean - mu0) / (sigma / Math.sqrt(n));
    
    // Use jStat built-in tests
    // jStat.ztest(value, mean, sd, sides)
    // Note: sd in jStat.ztest refers to standard error (sigma/sqrt(n)) when value is a sample mean
    const stdError = sigma / Math.sqrt(n);
    const pValueTwoTail = jstat.ztest(mean, mu0, stdError, 2);
    
    // Since jStat.ztest logic is simple, let's stick to our robust manual calculation for p-values to ensure correct left/right labeling
    // But use jStat for CI as requested
    const ci = jstat.normalci(mean, alpha, sigma, n);

    return {
      z,
      pValueTwoTail,
      pValueLeft: jstat.normal.cdf(z, 0, 1),
      pValueRight: 1 - jstat.normal.cdf(z, 0, 1),
      lowerCI: ci[0],
      upperCI: ci[1]
    };
  }

  /**
   * Two-Sample Z-Test
   * H0: mu1 - mu2 = diff (usually 0)
   * @param {object} params
   * @param {number} params.mean1 - Sample 1 mean
   * @param {number} params.n1 - Sample 1 size
   * @param {number} params.sigma1 - Population 1 std dev
   * @param {number} params.mean2 - Sample 2 mean
   * @param {number} params.n2 - Sample 2 size
   * @param {number} params.sigma2 - Population 2 std dev
   * @param {number} [params.diff=0] - Hypothesized difference
   * @param {number} [params.alpha=0.05] - Significance level
   * @returns {object} { z, pValueTwoTail, pValueLeft, pValueRight, lowerCI, upperCI }
   */
  static zTestTwoSample({ mean1, n1, sigma1, mean2, n2, sigma2, diff = 0, alpha = 0.05 }) {
    const error = this._validateParams({ n: n1, sigma: sigma1 }) || this._validateParams({ n: n2, sigma: sigma2 });
    if (error) return error;

    const se = Math.sqrt((sigma1 * sigma1 / n1) + (sigma2 * sigma2 / n2));
    const z = ((mean1 - mean2) - diff) / se;
    
    const meanDiff = mean1 - mean2;
    const zCrit = jstat.normal.inv(1 - alpha / 2, 0, 1);
    const marginOfError = zCrit * se;

    return {
      z,
      pValueTwoTail: 2 * (1 - jstat.normal.cdf(Math.abs(z), 0, 1)),
      pValueLeft: jstat.normal.cdf(z, 0, 1),
      pValueRight: 1 - jstat.normal.cdf(z, 0, 1),
      lowerCI: meanDiff - marginOfError,
      upperCI: meanDiff + marginOfError
    };
  }

  /**
   * One-Sample t-Test
   * H0: mu = mu0
   * @param {object} params
   * @param {number} params.mean - Sample mean
   * @param {number} params.n - Sample size
   * @param {number} params.s - Sample standard deviation
   * @param {number} params.mu0 - Hypothesized mean
   * @param {number} [params.alpha=0.05] - Significance level
   * @returns {object} { t, df, pValueTwoTail, pValueLeft, pValueRight, lowerCI, upperCI }
   */
  static tTestOneSample({ mean, n, s, mu0, alpha = 0.05 }) {
    const error = this._validateParams({ n, s });
    if (error) return error;
    if (n <= 1) return { error: 'invalid_n', message: 'Sample size must be > 1 for t-test' };

    const t = (mean - mu0) / (s / Math.sqrt(n));
    const df = n - 1;
    
    // Use built-in CI
    const ci = jstat.tci(mean, alpha, s, n);

    return {
      t,
      df,
      pValueTwoTail: jstat.ttest(mean, mu0, s, n, 2),
      pValueLeft: jstat.studentt.cdf(t, df),
      pValueRight: 1 - jstat.studentt.cdf(t, df),
      lowerCI: ci[0],
      upperCI: ci[1]
    };
  }

  /**
   * Two-Sample t-Test (Independent)
   * H0: mu1 - mu2 = diff
   * @param {object} params
   * @param {number} params.mean1
   * @param {number} params.n1
   * @param {number} params.s1
   * @param {number} params.mean2
   * @param {number} params.n2
   * @param {number} params.s2
   * @param {boolean} [params.equalVariances=false] - Pooled or Welch's
   * @param {number} [params.diff=0]
   * @param {number} [params.alpha=0.05]
   * @returns {object} { t, df, pValueTwoTail, pValueLeft, pValueRight, lowerCI, upperCI }
   */
  static tTestTwoSample({ mean1, n1, s1, mean2, n2, s2, equalVariances = false, diff = 0, alpha = 0.05 }) {
    const error = this._validateParams({ n: n1, s: s1 }) || this._validateParams({ n: n2, s: s2 });
    if (error) return error;
    if (n1 <= 1 || n2 <= 1) return { error: 'invalid_n', message: 'Sample sizes must be > 1' };

    let t, df, se;
    if (equalVariances) {
      // Pooled variance
      const sp2 = ((n1 - 1) * s1 * s1 + (n2 - 1) * s2 * s2) / (n1 + n2 - 2);
      se = Math.sqrt(sp2 * (1 / n1 + 1 / n2));
      df = n1 + n2 - 2;
    } else {
      // Welch's t-test
      se = Math.sqrt((s1 * s1 / n1) + (s2 * s2 / n2));
      const num = Math.pow((s1 * s1 / n1) + (s2 * s2 / n2), 2);
      const den = (Math.pow(s1 * s1 / n1, 2) / (n1 - 1)) + (Math.pow(s2 * s2 / n2, 2) / (n2 - 1));
      df = num / den;
    }

    t = ((mean1 - mean2) - diff) / se;
    
    // CI for difference
    const meanDiff = mean1 - mean2;
    const tCrit = jstat.studentt.inv(1 - alpha / 2, df);
    const marginOfError = tCrit * se;

    return {
      t,
      df,
      pValueTwoTail: 2 * (1 - jstat.studentt.cdf(Math.abs(t), df)),
      pValueLeft: jstat.studentt.cdf(t, df),
      pValueRight: 1 - jstat.studentt.cdf(t, df),
      lowerCI: meanDiff - marginOfError,
      upperCI: meanDiff + marginOfError
    };
  }

  /**
   * Paired t-Test
   * H0: mu_d = 0 (Mean difference is 0)
   * @param {number[]} sample1 - Array of data 1
   * @param {number[]} sample2 - Array of data 2
   * @param {number} [alpha=0.05]
   * @returns {object} { t, df, pValueTwoTail, pValueLeft, pValueRight, meanDiff, sDiff, lowerCI, upperCI }
   */
  static tTestPaired(sample1, sample2, alpha = 0.05) {
    if (!sample1 || !sample2 || sample1.length !== sample2.length || sample1.length <= 1) {
      return { error: 'invalid_samples', message: 'Samples must be same length and > 1' };
    }
    
    const diffs = [];
    for (let i = 0; i < sample1.length; i++) {
      diffs.push(sample1[i] - sample2[i]);
    }
    
    const stats = this.getStats(diffs);
    const result = this.tTestOneSample({ 
      mean: stats.mean, 
      n: stats.n, 
      s: stats.s, 
      mu0: 0,
      alpha
    });
    
    return { ...result, meanDiff: stats.mean, sDiff: stats.s };
  }

  /**
   * Two-Sample F-Test for Equality of Variances
   * H0: sigma1^2 = sigma2^2
   * @param {object} params
   * @param {number} params.s1 - Sample 1 std dev
   * @param {number} params.n1 - Sample 1 size
   * @param {number} params.s2 - Sample 2 std dev
   * @param {number} params.n2 - Sample 2 size
   * @returns {object} { f, df1, df2, pValueTwoTail, pValueLeft, pValueRight }
   */
  static fTestTwoSample({ s1, n1, s2, n2 }) {
    const error = this._validateParams({ n: n1, s: s1 }) || this._validateParams({ n: n2, s: s2 });
    if (error) return error;
    if (n1 <= 1 || n2 <= 1) return { error: 'invalid_n', message: 'Sample sizes must be > 1' };
    
    const var1 = s1 * s1;
    const var2 = s2 * s2;
    const f = var1 / var2;
    const df1 = n1 - 1;
    const df2 = n2 - 1;
    
    // jStat.ftest returns p-value
    const pValueRight = jstat.ftest(f, df1, df2);
    const cdf = jstat.centralF.cdf(f, df1, df2);
    
    return {
      f,
      df1,
      df2,
      pValueLeft: cdf,
      pValueRight,
      pValueTwoTail: 2 * Math.min(cdf, 1 - cdf)
    };
  }

  /**
   * Chi-Square Goodness of Fit Test
   * H0: Observed distribution fits Expected distribution
   * @param {number[]} observed - Array of observed frequencies
   * @param {number[]} expected - Array of expected frequencies (or probabilities)
   * @returns {object} { chiSquare, df, pValue }
   */
  static chiSquareGoodnessOfFit(observed, expected) {
    if (!observed || !expected || observed.length !== expected.length || observed.length < 2) {
      return { error: 'invalid_data' };
    }

    const n = jstat.sum(observed);
    const expectedSum = jstat.sum(expected);
    
    // If expected values are probabilities (sum approx 1), convert to counts
    let expectedCounts = expected;
    if (Math.abs(expectedSum - 1) < 0.01 && Math.abs(expectedSum - n) > 1) {
      expectedCounts = expected.map(p => p * n);
    } else if (Math.abs(expectedSum - n) > 1) {
       const scale = n / expectedSum;
       expectedCounts = expected.map(e => e * scale);
    }

    let chiSquare = 0;
    for (let i = 0; i < observed.length; i++) {
      const o = observed[i];
      const e = expectedCounts[i];
      if (e <= 0) return { error: 'expected_zero' };
      chiSquare += Math.pow(o - e, 2) / e;
    }

    const df = observed.length - 1;
    const pValue = 1 - jstat.chisquare.cdf(chiSquare, df);

    return {
      chiSquare,
      df,
      pValue
    };
  }

  /**
   * Simple Linear Regression t-Test for Slope
   * H0: beta1 = 0 (No linear relationship)
   * @param {number[]} x - Independent variable
   * @param {number[]} y - Dependent variable
   * @param {number} [alpha=0.05] - Significance level
   * @returns {object} { slope, intercept, r, rSquared, t, df, pValueTwoTail, lowerCISlope, upperCISlope }
   */
  static linearRegression(x, y, alpha = 0.05) {
    if (!x || !y || x.length !== y.length || x.length <= 2) {
      return { error: 'invalid_data', message: 'Need at least 3 points for regression' };
    }

    const n = x.length;
    const meanX = jstat.mean(x);
    const meanY = jstat.mean(y);
    
    let sxx = 0;
    let sxy = 0;
    let syy = 0;

    for (let i = 0; i < n; i++) {
      const dx = x[i] - meanX;
      const dy = y[i] - meanY;
      sxx += dx * dx;
      sxy += dx * dy;
      syy += dy * dy;
    }

    if (sxx === 0) return { error: 'zero_variance_x', message: 'X values must vary' };

    const slope = sxy / sxx;
    const intercept = meanY - slope * meanX;
    
    // Correlation coefficient r
    const r = sxy / Math.sqrt(sxx * syy);
    const rSquared = r * r;

    // Standard error of slope
    const sse = syy - slope * sxy;
    const mse = sse / (n - 2);
    
    let t, pValueTwoTail, seSlope;
    if (mse <= 1e-10) {
        // Perfect fit
        t = Infinity;
        pValueTwoTail = 0;
        seSlope = 0;
    } else {
        seSlope = Math.sqrt(mse / sxx);
        t = slope / seSlope;
        const df = n - 2;
        pValueTwoTail = jstat.ttest(0, slope, seSlope, 1, 2); // Testing if slope = 0. Here 'mean'=slope, 'popMean'=0, 'sd'=seSlope, n=1 (standard normal approx? No, t-dist).
        // Wait, jstat.ttest uses n for df=n-1? 
        // jStat.ttest(val, mean, sd, n, sides) => t = (val - mean) / (sd / sqrt(n))
        // We have t = slope / seSlope.
        // If we set val=slope, mean=0, sd=seSlope, n=1 => t = slope/seSlope. 
        // But df would be n-1 = 0? 
        // So jstat.ttest might not work directly for regression slope t-test where df = n-2.
        // We'll stick to manual t calculation for regression.
        pValueTwoTail = 2 * (1 - jstat.studentt.cdf(Math.abs(t), df));
    }
    
    // Confidence Interval for Slope
    let lowerCISlope = null;
    let upperCISlope = null;
    if (mse > 1e-10) {
        const df = n - 2;
        const tCrit = jstat.studentt.inv(1 - alpha / 2, df);
        const marginOfError = tCrit * seSlope;
        lowerCISlope = slope - marginOfError;
        upperCISlope = slope + marginOfError;
    }

    return {
      slope,
      intercept,
      r,
      rSquared,
      t,
      df: n - 2,
      pValueTwoTail,
      lowerCISlope,
      upperCISlope
    };
  }

  /**
   * Chi-Square Test of Independence
   * @param {number[][]} observedMatrix - 2D array of observed frequencies
   * @returns {object} { chiSquare, df, pValue }
   */
  static chiSquareIndependence(observedMatrix) {
    if (!observedMatrix || observedMatrix.length < 2 || observedMatrix[0].length < 2) {
      return { error: 'invalid_matrix', message: 'Matrix must be at least 2x2' };
    }

    const rows = observedMatrix.length;
    const cols = observedMatrix[0].length;
    const rowTotals = new Array(rows).fill(0);
    const colTotals = new Array(cols).fill(0);
    let grandTotal = 0;

    for (let i = 0; i < rows; i++) {
      for (let j = 0; j < cols; j++) {
        const val = observedMatrix[i][j];
        if (val < 0) return { error: 'negative_value', message: 'Observed counts cannot be negative' };
        rowTotals[i] += val;
        colTotals[j] += val;
        grandTotal += val;
      }
    }

    let chiSquare = 0;
    for (let i = 0; i < rows; i++) {
      for (let j = 0; j < cols; j++) {
        const expected = (rowTotals[i] * colTotals[j]) / grandTotal;
        if (expected > 0) {
            const obs = observedMatrix[i][j];
            chiSquare += Math.pow(obs - expected, 2) / expected;
        }
      }
    }

    const df = (rows - 1) * (cols - 1);
    const pValue = 1 - jstat.chisquare.cdf(chiSquare, df);

    return {
      chiSquare,
      df,
      pValue
    };
  }
}

export default HypothesisEngine;
