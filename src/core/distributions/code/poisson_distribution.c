#include "poisson_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

// Parameter names for Poisson distribution
static const char* poisson_param_names[] = {"lambda"};

/**
 * @brief Poisson distribution PDF calculation
 * Formula: P(X = k) = (lambda^k * e^(-lambda)) / k!
 * where k is the number of events and lambda is the rate parameter
 */
double poisson_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 1) {
        return NAN;
    }
    
    if (!poisson_validate_params(params, param_count)) {
        return NAN;
    }
    
    double lambda = params[0]; // rate parameter
    
    if (!is_finite_number(x)) {
        return NAN;
    }
    
    // Check if x is a non-negative integer
    if (x < 0.0 || floor(x) != x) {
        return 0.0;
    }
    
    int k = (int)x; // number of events
    
    // Handle edge cases
    if (lambda == 0.0) {
        return (k == 0) ? 1.0 : 0.0;
    }
    
    // For very small lambda and k=0, use direct calculation
    if (k == 0) {
        return safe_exp(-lambda);
    }
    
    // Calculate PDF using log space for numerical stability
    // log(P(X = k)) = k*log(lambda) - lambda - log(k!)
    double log_prob = k * safe_log(lambda) - lambda - log_factorial(k);
    
    return safe_exp(log_prob);
}

/**
 * @brief Poisson distribution CDF calculation
 * Formula: P(X ≤ k) = sum_{i=0}^{k} P(X = i)
 * For large lambda, uses normal approximation with continuity correction
 */
double poisson_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 1) {
        return NAN;
    }
    
    if (!poisson_validate_params(params, param_count)) {
        return NAN;
    }
    
    double lambda = params[0]; // rate parameter
    
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
    if (lambda == 0.0) {
        return 1.0; // All probability is at k=0
    }
    
    // For large lambda (>= 30), use normal approximation with continuity correction
    if (lambda >= 30.0) {
        double mean = lambda;
        double std_dev = sqrt(lambda);
        
        // Apply continuity correction: P(X <= k) ≈ P(Z <= (k + 0.5 - mean) / std_dev)
        double z = (k + 0.5 - mean) / std_dev;
        
        // Use error function to compute normal CDF
        return 0.5 * (1.0 + error_function(z / M_SQRT2));
    }
    
    // For smaller lambda, use direct summation with recurrence relation
    double cdf = 0.0;
    double current_pdf = safe_exp(-lambda); // P(X = 0)
    cdf += current_pdf;
    
    // Use recurrence relation: P(X = i) = P(X = i-1) * lambda / i
    for (int i = 1; i <= k; i++) {
        current_pdf *= lambda / (double)i;
        cdf += current_pdf;
        
        // Early termination if PDF becomes negligible
        if (current_pdf < 1e-15) {
            break;
        }
    }
    
    return cdf;
}

/**
 * @brief Validate Poisson distribution parameters
 * Parameter: lambda (rate parameter) must be positive
 */
int poisson_validate_params(double* params, int param_count) {
    if (!params || param_count != 1) {
        return 0;
    }
    
    double lambda = params[0];
    
    // lambda must be finite and positive
    if (!is_finite_number(lambda) || lambda <= 0.0) {
        return 0;
    }
    
    return 1;
}

/**
 * @brief Get the Poisson distribution interface
 */
const distribution_t* get_poisson_distribution(void) {
    static const distribution_t poisson_dist = {
        .pdf = poisson_pdf,
        .cdf = poisson_cdf,
        .validate_params = poisson_validate_params,
        .distribution_name = "Poisson",
        .param_count = 1,
        .param_names = poisson_param_names
    };
    
    return &poisson_dist;
}