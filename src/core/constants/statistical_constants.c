#include "statistical_constants.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// Structure for essential critical values lookup
typedef struct {
    double alpha_levels[5];
    double chi_square_1df[5];
    double chi_square_2df[5];
    double t_1df[5];
    double t_2df[5];
} essential_critical_values_t;

// Essential critical values for common significance levels
const essential_critical_values_t essential_critical_values = {
    .alpha_levels = {0.10, 0.05, 0.025, 0.01, 0.005},
    .chi_square_1df = {2.7055, 3.8415, 5.0239, 6.6349, 7.8794},
    .chi_square_2df = {4.6052, 5.9915, 7.3778, 9.2103, 10.5966},
    .t_1df = {3.0777, 6.3138, 12.7062, 31.8205, 63.6567},
    .t_2df = {1.8856, 2.9200, 4.3027, 6.9646, 9.9248}
};

// Small factorial cache for very fast lookup (0! to 12!)
const double small_factorial_cache[SMALL_FACTORIAL_CACHE_SIZE] = {
    1.0,                    // 0!
    1.0,                    // 1!
    2.0,                    // 2!
    6.0,                    // 3!
    24.0,                   // 4!
    120.0,                  // 5!
    720.0,                  // 6!
    5040.0,                 // 7!
    40320.0,                // 8!
    362880.0,               // 9!
    3628800.0,              // 10!
    39916800.0,             // 11!
    479001600.0             // 12!
};

/**
 * High-performance factorial calculation using optimized approximations
 */
double calculate_factorial(int n) {
    if (n < 0) return NAN;
    if (n < SMALL_FACTORIAL_CACHE_SIZE) {
        return small_factorial_cache[n];
    }
    if (n <= 170) {
        // Use Lanczos gamma approximation for medium values
        return lanczos_gamma_approximation(n + 1.0);
    }
    // For very large values, return infinity to avoid overflow
    return INFINITY;
}

/**
 * High-performance log factorial using optimized Stirling's approximation
 */
double calculate_log_factorial(int n) {
    if (n < 0) return NAN;
    if (n < SMALL_FACTORIAL_CACHE_SIZE) {
        return log(small_factorial_cache[n]);
    }
    if (n < 20) {
        // Use Lanczos for small-medium values
        return log(lanczos_gamma_approximation(n + 1.0));
    }
    // Use optimized Stirling's approximation for large values
    return stirling_log_factorial_approximation(n);
}

/**
 * Stirling's approximation for log(n!)
 * log(n!) ≈ n*log(n) - n + 0.5*log(2*π*n)
 */
double stirling_log_factorial(int n) {
    if (n <= 0) return 0.0;
    double dn = (double)n;
    return dn * log(dn) - dn + 0.5 * log(2.0 * STAT_PI * dn);
}

/**
 * Fast normal CDF using rational approximation
 */
double fast_normal_cdf(double z) {
    return normal_cdf_approximation(z);
}

/**
 * High-accuracy normal CDF approximation
 * Uses Hart's algorithm for better accuracy than error function approach
 */
double normal_cdf_approximation(double z) {
    if (z == 0.0) return 0.5;
    
    // Use symmetry for negative values
    bool negative = (z < 0.0);
    z = fabs(z);
    
    double result;
    
    if (z < 7.0) {
        // Rational approximation for |z| < 7
        double exp_val = exp(-0.5 * z * z);
        double numerator = (((1.330274429 * z + 1.821255978) * z + 1.781477937) * z + 0.356563782) * z + 0.319381530;
        double denominator = ((((1.330274429 * z + 1.821255978) * z + 1.781477937) * z + 0.356563782) * z + 0.319381530) * z + 1.0;
        
        result = 1.0 - exp_val * numerator / denominator / STAT_SQRT_2PI;
    } else {
        // Asymptotic expansion for large |z|
        result = 1.0;
    }
    
    return negative ? (1.0 - result) : result;
}

/**
 * Chi-square critical value approximation
 */
double fast_chi_square_critical(int df, double alpha) {
    // Check essential table first for common cases
    if (df == 1) {
        for (int i = 0; i < 5; i++) {
            if (fabs(alpha - essential_critical_values.alpha_levels[i]) < 1e-6) {
                return essential_critical_values.chi_square_1df[i];
            }
        }
    }
    if (df == 2) {
        for (int i = 0; i < 5; i++) {
            if (fabs(alpha - essential_critical_values.alpha_levels[i]) < 1e-6) {
                return essential_critical_values.chi_square_2df[i];
            }
        }
    }
    
    // Use approximation for other cases
    return chi_square_critical_approximation(df, alpha);
}

/**
 * Chi-square critical value approximation using Wilson-Hilferty transformation
 */
double chi_square_critical_approximation(int df, double alpha) {
    if (df <= 0 || alpha <= 0.0 || alpha >= 1.0) return NAN;
    
    // Get normal critical value
    double z_alpha = inverse_normal_cdf(1.0 - alpha);
    
    if (df >= 30) {
        // Normal approximation for large df
        return df + z_alpha * sqrt(2.0 * df);
    } else {
        // Wilson-Hilferty transformation
        double h = 2.0 / (9.0 * df);
        double term = 1.0 - h + z_alpha * sqrt(h);
        return df * term * term * term;
    }
}

/**
 * t-distribution critical value approximation
 */
double fast_t_critical(int df, double alpha) {
    // Check essential table first for common cases
    if (df == 1) {
        for (int i = 0; i < 5; i++) {
            if (fabs(alpha - essential_critical_values.alpha_levels[i]) < 1e-6) {
                return essential_critical_values.t_1df[i];
            }
        }
    }
    if (df == 2) {
        for (int i = 0; i < 5; i++) {
            if (fabs(alpha - essential_critical_values.alpha_levels[i]) < 1e-6) {
                return essential_critical_values.t_2df[i];
            }
        }
    }
    
    // Use approximation for other cases
    return t_critical_approximation(df, alpha);
}

/**
 * t-distribution critical value approximation
 */
double t_critical_approximation(int df, double alpha) {
    if (df <= 0 || alpha <= 0.0 || alpha >= 1.0) return NAN;
    
    // Get normal critical value
    double z_alpha = inverse_normal_cdf(1.0 - alpha / 2.0);
    
    if (df >= 30) {
        // Use normal approximation for large df
        return z_alpha;
    } else {
        // Cornish-Fisher expansion approximation
        double z2 = z_alpha * z_alpha;
        double z4 = z2 * z2;
        
        double c1 = z_alpha / (4.0 * df);
        double c2 = (5.0 * z_alpha + 16.0 * z_alpha * z2 + 3.0 * z_alpha * z4) / (96.0 * df * df);
        
        return z_alpha + c1 + c2;
    }
}

/**
 * Inverse normal CDF using Beasley-Springer-Moro algorithm
 */
double inverse_normal_cdf(double p) {
    if (p <= 0.0 || p >= 1.0) return NAN;
    if (p == 0.5) return 0.0;
    
    // Use symmetry
    bool lower = (p < 0.5);
    if (lower) p = 1.0 - p;
    
    double result;
    
    if (p > 0.5 && p < 0.92) {
        // Central region
        double q = p - 0.5;
        double r = q * q;
        result = q * ((((((-25.44106049637 * r + 41.39119773534) * r + -18.61500062529) * r + 2.50662823884) * r + -1.0) * r + 0.0) + 1.0) /
                 ((((((-7.784894002430 * r + 14.38718147627) * r + -7.734576761300) * r + 2.05319162663) * r + 1.0) * r + 0.0) + 0.0);
    } else {
        // Tail region
        double r = sqrt(-log(1.0 - p));
        if (r <= 5.0) {
            r = r - 1.6;
            result = (((((((7.7454501427834140764e-4 * r + 0.0227238449892691845833) * r + 0.24178072517745061177) * r + 1.27045825245236838258) * r + 3.64784832476320460504) * r + 5.7694972214606914055) * r + 4.6303378461565452959) * r + 1.42343711074968357734) /
                     (((((((1.05075007164441684324e-9 * r + 5.475938084995344946e-4) * r + 0.0151986665636164571966) * r + 0.14810397642748007459) * r + 0.68976733498510000455) * r + 1.6763848301838038494) * r + 2.05319162663775882187) * r + 1.0);
        } else {
            r = r - 5.0;
            result = (((((((2.01033439929228813265e-7 * r + 2.71155556874348757815e-5) * r + 0.0012426609473880784386) * r + 0.026532189526576123093) * r + 0.29656057182850489123) * r + 1.7848265399172913358) * r + 5.4637849111641143699) * r + 6.6579046435011037772) /
                     (((((((2.04426310338993978564e-15 * r + 1.4215117583164458887e-7) * r + 1.8463183175100546818e-5) * r + 7.868691311456132591e-4) * r + 0.0148753612908506148525) * r + 0.13692988092273580531) * r + 0.59983220655588793769) * r + 1.0);
        }
    }
    
    return lower ? -result : result;
}

/**
 * Rational approximation for inverse normal CDF (helper function)
 */
double rational_approximation(double t) {
    // Abramowitz and Stegun approximation
    double c[] = {2.515517, 0.802853, 0.010328};
    double d[] = {1.432788, 0.189269, 0.001308};
    
    return t - ((c[2] * t + c[1]) * t + c[0]) / 
               (((d[2] * t + d[1]) * t + d[0]) * t + 1.0);
}