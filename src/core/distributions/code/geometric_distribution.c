#include "geometric_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

// Parameter names for Geometric distribution
static const char* geometric_param_names[] = {"p"};

/**
 * @brief Geometric distribution PDF calculation
 * Formula: P(X = k) = (1-p)^(k-1) * p for k = 1, 2, 3, ...
 * Note: Using the "number of trials until first success" definition
 */
double geometric_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 1) {
        return NAN;
    }
    
    if (!geometric_validate_params(params, param_count)) {
        return NAN;
    }
    
    double p = params[0]; // probability of success
    
    if (!is_finite_number(x)) {
        return NAN;
    }
    
    // Check if x is a positive integer
    if (x < 1.0 || floor(x) != x) {
        return 0.0;
    }
    
    int k = (int)x;
    
    // Handle edge cases
    if (p == 1.0) {
        return (k == 1) ? 1.0 : 0.0;
    }
    
    // Calculate PDF: (1-p)^(k-1) * p
    // Use log space for numerical stability with large k
    double log_prob = (k - 1) * safe_log(1.0 - p) + safe_log(p);
    
    return safe_exp(log_prob);
}

/**
 * @brief Geometric distribution CDF calculation
 * Formula: P(X ≤ k) = 1 - (1-p)^k for k = 1, 2, 3, ...
 */
double geometric_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 1) {
        return NAN;
    }
    
    if (!geometric_validate_params(params, param_count)) {
        return NAN;
    }
    
    double p = params[0]; // probability of success
    
    if (!is_finite_number(x)) {
        if (x == -INFINITY) return 0.0;
        if (x == INFINITY) return 1.0;
        return NAN;
    }
    
    // CDF is 0 for x < 1
    if (x < 1.0) {
        return 0.0;
    }
    
    // For discrete distribution, use floor of x
    int k = (int)floor(x);
    
    // Handle edge case
    if (p == 1.0) {
        return 1.0;
    }
    
    // Calculate CDF: 1 - (1-p)^k
    // Use log space for numerical stability with large k
    double log_complement = k * safe_log(1.0 - p);
    
    return 1.0 - safe_exp(log_complement);
}

/**
 * @brief Validate Geometric distribution parameters
 * Parameter: p (probability of success) must be in (0, 1]
 */
int geometric_validate_params(double* params, int param_count) {
    if (!params || param_count != 1) {
        return 0;
    }
    
    double p = params[0];
    
    // p must be finite, positive, and at most 1
    if (!is_finite_number(p) || p <= 0.0 || p > 1.0) {
        return 0;
    }
    
    return 1;
}

/**
 * @brief Get the Geometric distribution interface
 */
const distribution_t* get_geometric_distribution(void) {
    static const distribution_t geometric_dist = {
        .pdf = geometric_pdf,
        .cdf = geometric_cdf,
        .validate_params = geometric_validate_params,
        .distribution_name = "Geometric",
        .param_count = 1,
        .param_names = geometric_param_names
    };
    
    return &geometric_dist;
}