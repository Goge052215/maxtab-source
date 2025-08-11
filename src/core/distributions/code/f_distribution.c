#include "f_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

// Parameter names for F-distribution
static const char* f_param_names[] = {"numerator_df", "denominator_df"};

// Forward declarations for helper functions
static double incomplete_beta_regularized_f(double a, double b, double x);
static double continued_fraction_beta_f(double a, double b, double x);

/**
 * @brief F-distribution PDF calculation
 * Formula: f(x) = [Γ((ν₁+ν₂)/2) / (Γ(ν₁/2)Γ(ν₂/2))] * (ν₁/ν₂)^(ν₁/2) * x^(ν₁/2-1) * (1 + (ν₁/ν₂)x)^(-(ν₁+ν₂)/2)
 */
double f_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!f_validate_params(params, param_count)) {
        return NAN;
    }
    
    double nu1 = params[0]; // numerator degrees of freedom
    double nu2 = params[1]; // denominator degrees of freedom
    
    if (!is_finite_number(x)) {
        if (x == INFINITY || x == -INFINITY) return 0.0;
        return NAN;
    }
    
    // PDF is 0 for negative x
    if (x <= 0.0) {
        return 0.0;
    }
    
    // Calculate PDF using log space for numerical stability
    double half_nu1 = nu1 / 2.0;
    double half_nu2 = nu2 / 2.0;
    double half_sum = (nu1 + nu2) / 2.0;
    
    // Log of normalization constant
    double log_norm = log_gamma_function(half_sum) - log_gamma_function(half_nu1) - log_gamma_function(half_nu2);
    
    // Log of (ν₁/ν₂)^(ν₁/2)
    double log_ratio_power = half_nu1 * safe_log(nu1 / nu2);
    
    // Log of x^(ν₁/2-1)
    double log_x_power = (half_nu1 - 1.0) * safe_log(x);
    
    // Log of (1 + (ν₁/ν₂)x)^(-(ν₁+ν₂)/2)
    double log_denominator = -half_sum * safe_log(1.0 + (nu1 / nu2) * x);
    
    return safe_exp(log_norm + log_ratio_power + log_x_power + log_denominator);
}

/**
 * @brief F-distribution CDF calculation
 * Uses the relationship with the incomplete beta function
 */
double f_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!f_validate_params(params, param_count)) {
        return NAN;
    }
    
    double nu1 = params[0]; // numerator degrees of freedom
    double nu2 = params[1]; // denominator degrees of freedom
    
    if (!is_finite_number(x)) {
        if (x == -INFINITY) return 0.0;
        if (x == INFINITY) return 1.0;
        return NAN;
    }
    
    // CDF is 0 for negative or zero x
    if (x <= 0.0) {
        return 0.0;
    }
    
    // Use the relationship with incomplete beta function
    // F(x) = I_z(ν₁/2, ν₂/2) where z = (ν₁x)/(ν₁x + ν₂)
    double z = (nu1 * x) / (nu1 * x + nu2);
    
    return incomplete_beta_regularized_f(nu1 / 2.0, nu2 / 2.0, z);
}

/**
 * @brief Regularized incomplete beta function I_x(a,b) for F-distribution
 */
static double incomplete_beta_regularized_f(double a, double b, double x) {
    if (x <= 0.0) return 0.0;
    if (x >= 1.0) return 1.0;
    if (a <= 0.0 || b <= 0.0) return NAN;
    
    // Use continued fraction expansion
    double bt = safe_exp(log_gamma_function(a + b) - log_gamma_function(a) - log_gamma_function(b) + 
                        a * safe_log(x) + b * safe_log(1.0 - x));
    
    if (x < (a + 1.0) / (a + b + 2.0)) {
        // Use continued fraction directly
        return bt * continued_fraction_beta_f(a, b, x) / a;
    } else {
        // Use symmetry relation
        return 1.0 - bt * continued_fraction_beta_f(b, a, 1.0 - x) / b;
    }
}

/**
 * @brief Continued fraction for incomplete beta function for F-distribution
 */
static double continued_fraction_beta_f(double a, double b, double x) {
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
 * @brief Validate F-distribution parameters
 * Parameters: numerator_df, denominator_df (both positive real numbers)
 */
int f_validate_params(double* params, int param_count) {
    if (!params || param_count != 2) {
        return 0;
    }
    
    double nu1 = params[0];
    double nu2 = params[1];
    
    // Both degrees of freedom must be positive and finite
    if (!is_finite_number(nu1) || nu1 <= 0.0) {
        return 0;
    }
    
    if (!is_finite_number(nu2) || nu2 <= 0.0) {
        return 0;
    }
    
    return 1;
}

/**
 * @brief Get the F-distribution interface
 */
const distribution_t* get_f_distribution(void) {
    static const distribution_t f_dist = {
        .pdf = f_pdf,
        .cdf = f_cdf,
        .validate_params = f_validate_params,
        .distribution_name = "F-distribution",
        .param_count = 2,
        .param_names = f_param_names
    };
    
    return &f_dist;
}