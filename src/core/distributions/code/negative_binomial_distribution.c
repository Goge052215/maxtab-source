#include "negative_binomial_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

// Parameter names for Negative Binomial distribution
static const char* negative_binomial_param_names[] = {"r", "p"};

/**
 * @brief Negative Binomial distribution PDF calculation
 * Formula: P(X = k) = C(k+r-1, k) * p^r * (1-p)^k
 * where k is the number of failures before the r-th success
 * Alternative formula: P(X = k) = C(k+r-1, r-1) * p^r * (1-p)^k
 */
double negative_binomial_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!negative_binomial_validate_params(params, param_count)) {
        return NAN;
    }
    
    int r = (int)params[0]; // number of successes
    double p = params[1];   // probability of success
    
    if (!is_finite_number(x)) {
        return NAN;
    }
    
    // Check if x is a non-negative integer
    if (x < 0.0 || floor(x) != x) {
        return 0.0;
    }
    
    int k = (int)x; // number of failures
    
    // Handle edge cases
    if (p == 1.0) {
        return (k == 0) ? 1.0 : 0.0; // If p=1, we succeed immediately, so k=0
    }
    
    // Calculate PDF using log space for numerical stability
    // log(P(X = k)) = log(C(k+r-1, k)) + r*log(p) + k*log(1-p)
    // Using C(k+r-1, k) = C(k+r-1, r-1)
    double log_prob = log_combination(k + r - 1, k) + 
                      r * safe_log(p) + 
                      k * safe_log(1.0 - p);
    
    return safe_exp(log_prob);
}

/**
 * @brief Negative Binomial distribution CDF calculation
 * Formula: P(X ≤ k) = sum_{i=0}^{k} P(X = i)
 * Can also be expressed using the incomplete beta function
 */
double negative_binomial_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!negative_binomial_validate_params(params, param_count)) {
        return NAN;
    }
    
    int r = (int)params[0]; // number of successes
    double p = params[1];   // probability of success
    
    if (!is_finite_number(x)) {
        if (x == -INFINITY) return 0.0;
        if (x == INFINITY) return 1.0;
        return NAN;
    }
    
    // For discrete distribution, use floor of x
    int k = (int)floor(x);
    
    // CDF is 0 for k < 0
    if (k < 0) {
        return 0.0;
    }
    
    // Handle edge case
    if (p == 1.0) {
        return 1.0; // All probability is at k=0
    }
    
    // For reasonable values of k, use direct summation
    // For very large k, the tail probability becomes negligible
    double cdf = 0.0;
    double current_pdf = negative_binomial_pdf(0.0, params, param_count);
    cdf += current_pdf;
    
    // Sum PDF values from 0 to k
    for (int i = 1; i <= k; i++) {
        // Use recurrence relation for efficiency:
        // P(X = i) = P(X = i-1) * (i + r - 1) * (1-p) / i
        current_pdf *= ((double)(i + r - 1) * (1.0 - p)) / (double)i;
        cdf += current_pdf;
        
        // Early termination if PDF becomes negligible
        if (current_pdf < 1e-15) {
            break;
        }
    }
    
    return cdf;
}

/**
 * @brief Validate Negative Binomial distribution parameters
 * Parameters: r (number of successes), p (probability of success)
 * Constraints: r > 0 (positive integer), 0 < p <= 1
 */
int negative_binomial_validate_params(double* params, int param_count) {
    if (!params || param_count != 2) {
        return 0;
    }
    
    double r_double = params[0];
    double p = params[1];
    
    // Both parameters must be finite
    if (!is_finite_number(r_double) || !is_finite_number(p)) {
        return 0;
    }
    
    // r must be a positive integer
    if (r_double <= 0.0 || floor(r_double) != r_double) {
        return 0;
    }
    
    // p must be in (0, 1]
    if (p <= 0.0 || p > 1.0) {
        return 0;
    }
    
    return 1;
}

/**
 * @brief Get the Negative Binomial distribution interface
 */
const distribution_t* get_negative_binomial_distribution(void) {
    static const distribution_t negative_binomial_dist = {
        .pdf = negative_binomial_pdf,
        .cdf = negative_binomial_cdf,
        .validate_params = negative_binomial_validate_params,
        .distribution_name = "Negative Binomial",
        .param_count = 2,
        .param_names = negative_binomial_param_names
    };
    
    return &negative_binomial_dist;
}