#include "t_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

// Parameter names for t-distribution
static const char* t_param_names[] = {"degrees_of_freedom"};

// Forward declarations for helper functions
static double incomplete_beta_regularized(double a, double b, double x);
static double continued_fraction_beta(double a, double b, double x);

/**
 * @brief Student's t-distribution PDF calculation
 * Formula: f(x) = Γ((ν+1)/2) / (√(νπ) * Γ(ν/2)) * (1 + x²/ν)^(-(ν+1)/2)
 */
double t_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 1) {
        return NAN;
    }
    
    if (!t_validate_params(params, param_count)) {
        return NAN;
    }
    
    double nu = params[0]; // degrees of freedom
    
    if (!is_finite_number(x)) {
        if (x == INFINITY || x == -INFINITY) return 0.0;
        return NAN;
    }
    
    // Calculate PDF using log space for numerical stability
    double half_nu = nu / 2.0;
    double half_nu_plus_1 = (nu + 1.0) / 2.0;
    
    // Log of normalization constant: log(Γ((ν+1)/2)) - log(√(νπ)) - log(Γ(ν/2))
    double log_norm = log_gamma_function(half_nu_plus_1) - 0.5 * safe_log(nu * M_PI_PRECISE) - log_gamma_function(half_nu);
    
    // Log of (1 + x²/ν)^(-(ν+1)/2)
    double log_power = -half_nu_plus_1 * safe_log(1.0 + (x * x) / nu);
    
    return safe_exp(log_norm + log_power);
}

/**
 * @brief Student's t-distribution CDF calculation
 * Uses the relationship with the incomplete beta function
 */
double t_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 1) {
        return NAN;
    }
    
    if (!t_validate_params(params, param_count)) {
        return NAN;
    }
    
    double nu = params[0]; // degrees of freedom
    
    if (!is_finite_number(x)) {
        if (x == -INFINITY) return 0.0;
        if (x == INFINITY) return 1.0;
        return NAN;
    }
    
    if (x == 0.0) {
        return 0.5;
    }
    
    // For large degrees of freedom, approximate with normal distribution
    if (nu > 100.0) {
        // Use error function approximation
        return 0.5 * (1.0 + error_function(x / M_SQRT2));
    }
    
    // Use the relationship: F(t) = 0.5 + (t/√(ν)) * Γ((ν+1)/2) / (√π * Γ(ν/2)) * ₂F₁(0.5, (ν+1)/2; 1.5; -t²/ν)
    // For simplicity, we'll use a series approximation
    
    double t_squared = x * x;
    double ratio = t_squared / (nu + t_squared);
    
    // Use incomplete beta function relationship
    // If t > 0: F(t) = 0.5 + 0.5 * I_ratio(0.5, nu/2)
    // If t < 0: F(t) = 0.5 - 0.5 * I_ratio(0.5, nu/2)
    
    double beta_result = incomplete_beta_regularized(0.5, nu / 2.0, ratio);
    
    if (x > 0.0) {
        return 0.5 + 0.5 * beta_result;
    } else {
        return 0.5 - 0.5 * beta_result;
    }
}

/**
 * @brief Regularized incomplete beta function I_x(a,b)
 * Used for t-distribution CDF calculation
 */
static double incomplete_beta_regularized(double a, double b, double x) {
    if (x <= 0.0) return 0.0;
    if (x >= 1.0) return 1.0;
    if (a <= 0.0 || b <= 0.0) return NAN;
    
    // Use continued fraction expansion
    double bt = safe_exp(log_gamma_function(a + b) - log_gamma_function(a) - log_gamma_function(b) + 
                        a * safe_log(x) + b * safe_log(1.0 - x));
    
    if (x < (a + 1.0) / (a + b + 2.0)) {
        // Use continued fraction directly
        return bt * continued_fraction_beta(a, b, x) / a;
    } else {
        // Use symmetry relation
        return 1.0 - bt * continued_fraction_beta(b, a, 1.0 - x) / b;
    }
}

/**
 * @brief Continued fraction for incomplete beta function
 */
static double continued_fraction_beta(double a, double b, double x) {
    const int max_iter = 200;
    const double eps = 1e-12;
    
    double qab = a + b;
    double qap = a + 1.0;
    double qam = a - 1.0;
    double c = 1.0;
    double d = 1.0 - qab * x / qap;
    
    if (fabs(d) < 1e-30) d = 1e-30;
    d = 1.0 / d;
    double h = d;
    
    for (int m = 1; m <= max_iter; m++) {
        int m2 = 2 * m;
        double aa = m * (b - m) * x / ((qam + m2) * (a + m2));
        d = 1.0 + aa * d;
        if (fabs(d) < 1e-30) d = 1e-30;
        c = 1.0 + aa / c;
        if (fabs(c) < 1e-30) c = 1e-30;
        d = 1.0 / d;
        h *= d * c;
        
        aa = -(a + m) * (qab + m) * x / ((a + m2) * (qap + m2));
        d = 1.0 + aa * d;
        if (fabs(d) < 1e-30) d = 1e-30;
        c = 1.0 + aa / c;
        if (fabs(c) < 1e-30) c = 1e-30;
        d = 1.0 / d;
        double del = d * c;
        h *= del;
        
        if (fabs(del - 1.0) < eps) break;
    }
    
    return h;
}

/**
 * @brief Validate Student's t-distribution parameters
 * Parameters: degrees_of_freedom (positive real number)
 */
int t_validate_params(double* params, int param_count) {
    if (!params || param_count != 1) {
        return 0;
    }
    
    double nu = params[0];
    
    // Degrees of freedom must be positive and finite
    if (!is_finite_number(nu) || nu <= 0.0) {
        return 0;
    }
    
    return 1;
}

/**
 * @brief Get the Student's t-distribution interface
 */
const distribution_t* get_t_distribution(void) {
    static const distribution_t t_dist = {
        .pdf = t_pdf,
        .cdf = t_cdf,
        .validate_params = t_validate_params,
        .distribution_name = "t-distribution",
        .param_count = 1,
        .param_names = t_param_names
    };
    
    return &t_dist;
}