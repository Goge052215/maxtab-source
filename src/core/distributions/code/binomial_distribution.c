#include "binomial_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

// Parameter names for Binomial distribution
static const char* binomial_param_names[] = {"n", "p"};

/**
 * @brief Binomial distribution PDF calculation
 * Formula: P(X = k) = C(n,k) * p^k * (1-p)^(n-k)
 * where C(n,k) is the binomial coefficient "n choose k"
 */
double binomial_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!binomial_validate_params(params, param_count)) {
        return NAN;
    }
    
    int n = (int)params[0]; // number of trials
    double p = params[1];   // probability of success
    
    if (!is_finite_number(x)) {
        return NAN;
    }
    
    // Check if x is a non-negative integer
    if (x < 0.0 || floor(x) != x) {
        return 0.0;
    }
    
    int k = (int)x; // number of successes
    
    // Check if k is within valid range [0, n]
    if (k > n) {
        return 0.0;
    }
    
    // Handle edge cases
    if (p == 0.0) {
        return (k == 0) ? 1.0 : 0.0;
    }
    
    if (p == 1.0) {
        return (k == n) ? 1.0 : 0.0;
    }
    
    // Calculate PDF using log space for numerical stability
    // log(P(X = k)) = log(C(n,k)) + k*log(p) + (n-k)*log(1-p)
    double log_prob = log_combination(n, k) + 
                      k * safe_log(p) + 
                      (n - k) * safe_log(1.0 - p);
    
    return safe_exp(log_prob);
}

/**
 * @brief Binomial distribution CDF calculation
 * Formula: P(X ≤ k) = sum_{i=0}^{k} P(X = i)
 * Uses direct summation for small n, and normal approximation for large n
 */
double binomial_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!binomial_validate_params(params, param_count)) {
        return NAN;
    }
    
    int n = (int)params[0]; // number of trials
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
    
    // CDF is 1 for k >= n
    if (k >= n) {
        return 1.0;
    }
    
    // Handle edge cases
    if (p == 0.0) {
        return 1.0; // All probability is at k=0
    }
    
    if (p == 1.0) {
        return (k >= n) ? 1.0 : 0.0; // All probability is at k=n
    }
    
    // For large n, use normal approximation with continuity correction
    // if n*p*(1-p) >= 9 and both n*p >= 5 and n*(1-p) >= 5
    if (n >= 30 && n * p * (1.0 - p) >= 9.0 && n * p >= 5.0 && n * (1.0 - p) >= 5.0) {
        double mean = n * p;
        double variance = n * p * (1.0 - p);
        double std_dev = sqrt(variance);
        
        // Apply continuity correction: P(X <= k) ≈ P(Z <= (k + 0.5 - mean) / std_dev)
        double z = (k + 0.5 - mean) / std_dev;
        
        // Use error function to compute normal CDF
        return 0.5 * (1.0 + error_function(z / M_SQRT2));
    }
    
    // For smaller n, use direct summation
    double cdf = 0.0;
    for (int i = 0; i <= k; i++) {
        cdf += binomial_pdf((double)i, params, param_count);
    }
    
    return cdf;
}

/**
 * @brief Validate Binomial distribution parameters
 * Parameters: n (number of trials), p (probability of success)
 * Constraints: n >= 0 (integer), 0 <= p <= 1
 */
int binomial_validate_params(double* params, int param_count) {
    if (!params || param_count != 2) {
        return 0;
    }
    
    double n_double = params[0];
    double p = params[1];
    
    // Both parameters must be finite
    if (!is_finite_number(n_double) || !is_finite_number(p)) {
        return 0;
    }
    
    // n must be a non-negative integer
    if (n_double < 0.0 || floor(n_double) != n_double) {
        return 0;
    }
    
    // p must be a valid probability [0, 1]
    if (p < 0.0 || p > 1.0) {
        return 0;
    }
    
    return 1;
}

/**
 * @brief Get the Binomial distribution interface
 */
const distribution_t* get_binomial_distribution(void) {
    static const distribution_t binomial_dist = {
        .pdf = binomial_pdf,
        .cdf = binomial_cdf,
        .validate_params = binomial_validate_params,
        .distribution_name = "Binomial",
        .param_count = 2,
        .param_names = binomial_param_names
    };
    
    return &binomial_dist;
}