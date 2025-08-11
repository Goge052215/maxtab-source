#include "exponential_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

// Parameter names for Exponential distribution
static const char* exponential_param_names[] = {"lambda"};

/**
 * @brief Exponential distribution PDF calculation
 * Formula: f(x) = λ * exp(-λx) for x ≥ 0, 0 otherwise
 */
double exponential_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 1) {
        return NAN;
    }
    
    if (!exponential_validate_params(params, param_count)) {
        return NAN;
    }
    
    double lambda = params[0];
    
    if (!is_finite_number(x)) {
        if (x == INFINITY) return 0.0;
        if (x == -INFINITY) return 0.0;
        return NAN;
    }
    
    // PDF is 0 for negative x
    if (x < 0.0) {
        return 0.0;
    }
    
    // Calculate PDF: λ * exp(-λx)
    return lambda * safe_exp(-lambda * x);
}

/**
 * @brief Exponential distribution CDF calculation
 * Formula: F(x) = 1 - exp(-λx) for x ≥ 0, 0 otherwise
 */
double exponential_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 1) {
        return NAN;
    }
    
    if (!exponential_validate_params(params, param_count)) {
        return NAN;
    }
    
    double lambda = params[0];
    
    if (!is_finite_number(x)) {
        if (x == -INFINITY) return 0.0;
        if (x == INFINITY) return 1.0;
        return NAN;
    }
    
    // CDF is 0 for negative x
    if (x < 0.0) {
        return 0.0;
    }
    
    // Calculate CDF: 1 - exp(-λx)
    return 1.0 - safe_exp(-lambda * x);
}

/**
 * @brief Validate Exponential distribution parameters
 * Parameters: lambda (positive real number, rate parameter)
 */
int exponential_validate_params(double* params, int param_count) {
    if (!params || param_count != 1) {
        return 0;
    }
    
    double lambda = params[0];
    
    // Lambda must be positive and finite
    if (!is_finite_number(lambda) || lambda <= 0.0) {
        return 0;
    }
    
    return 1;
}

/**
 * @brief Get the Exponential distribution interface
 */
const distribution_t* get_exponential_distribution(void) {
    static const distribution_t exponential_dist = {
        .pdf = exponential_pdf,
        .cdf = exponential_cdf,
        .validate_params = exponential_validate_params,
        .distribution_name = "Exponential",
        .param_count = 1,
        .param_names = exponential_param_names
    };
    
    return &exponential_dist;
}