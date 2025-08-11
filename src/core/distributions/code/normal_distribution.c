#include "normal_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

// Parameter names for Normal distribution
static const char* normal_param_names[] = {"mean", "standard_deviation"};

/**
 * @brief Normal distribution PDF calculation
 * Formula: f(x) = (1/(σ√(2π))) * exp(-0.5 * ((x-μ)/σ)²)
 */
double normal_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!normal_validate_params(params, param_count)) {
        return NAN;
    }
    
    double mean = params[0];
    double std_dev = params[1];
    
    if (!is_finite_number(x)) {
        return NAN;
    }
    
    // Calculate standardized value
    double z = (x - mean) / std_dev;
    
    // Calculate PDF: (1/(σ√(2π))) * exp(-0.5 * z²)
    double coefficient = 1.0 / (std_dev * M_SQRT_2PI);
    double exponent = -0.5 * z * z;
    
    return coefficient * safe_exp(exponent);
}

/**
 * @brief Normal distribution CDF calculation
 * Formula: F(x) = 0.5 * (1 + erf((x-μ)/(σ√2)))
 */
double normal_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!normal_validate_params(params, param_count)) {
        return NAN;
    }
    
    double mean = params[0];
    double std_dev = params[1];
    
    if (!is_finite_number(x)) {
        if (x == -INFINITY) return 0.0;
        if (x == INFINITY) return 1.0;
        return NAN;
    }
    
    // Calculate standardized value
    double z = (x - mean) / (std_dev * M_SQRT2);
    
    // Calculate CDF: 0.5 * (1 + erf(z/√2))
    return 0.5 * (1.0 + error_function(z));
}

/**
 * @brief Validate Normal distribution parameters
 * Parameters: mean (any real number), standard_deviation (positive real number)
 */
int normal_validate_params(double* params, int param_count) {
    if (!params || param_count != 2) {
        return 0;
    }
    
    double mean = params[0];
    double std_dev = params[1];
    
    // Mean can be any finite real number
    if (!is_finite_number(mean)) {
        return 0;
    }
    
    // Standard deviation must be positive and finite
    if (!is_finite_number(std_dev) || std_dev <= 0.0) {
        return 0;
    }
    
    return 1;
}

/**
 * @brief Get the Normal distribution interface
 */
const distribution_t* get_normal_distribution(void) {
    static const distribution_t normal_dist = {
        .pdf = normal_pdf,
        .cdf = normal_cdf,
        .validate_params = normal_validate_params,
        .distribution_name = "Normal",
        .param_count = 2,
        .param_names = normal_param_names
    };
    
    return &normal_dist;
}