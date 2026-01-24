#include "chi_square_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

// Parameter names for Chi-Square distribution
static const char* chi_square_param_names[] = {"degrees_of_freedom"};

/**
 * @brief Regularized incomplete gamma function P(a,x) = γ(a,x)/Γ(a)
 * Used for Chi-Square CDF calculation
 */
static double regularized_incomplete_gamma_p(double a, double x) {
    if (x <= 0.0) return 0.0;
    if (a <= 0.0) return NAN;
    
    // For very large x relative to a, use asymptotic approximation
    if (x > a + 50.0) return 1.0;
    
    // Use series expansion for P(a,x) when x < a + 1
    if (x < a + 1.0) {
        double sum = 1.0;
        double term = 1.0;
        double ap = a;
        
        // Series: sum(x^n / Γ(a+n+1)) * Γ(a)
        for (int n = 1; n < 200; n++) {
            ap += 1.0;
            term *= x / ap;
            sum += term;
            if (fabs(term) < 1e-12) break;
        }
        
        // P(a,x) = (x^a * e^(-x) / Γ(a)) * sum
        double log_result = a * safe_log(x) - x - log_gamma_function(a) + safe_log(sum);
        
        if (log_result < -700.0) return 0.0;
        return safe_exp(log_result);
    } else {
        // Use continued fraction for x >= a + 1
        // Q(a,x) = 1 - P(a,x), so P(a,x) = 1 - Q(a,x)
        double b = x + 1.0 - a;
        double c = 1e30;
        double d = 1.0 / b;
        double h = d;
        
        for (int i = 1; i <= 200; i++) {
            double an = -i * (i - a);
            b += 2.0;
            d = an * d + b;
            if (fabs(d) < 1e-30) d = 1e-30;
            c = b + an / c;
            if (fabs(c) < 1e-30) c = 1e-30;
            d = 1.0 / d;
            double del = d * c;
            h *= del;
            if (fabs(del - 1.0) < 1e-12) break;
        }
        
        double log_q = a * safe_log(x) - x - log_gamma_function(a) + safe_log(h);
        if (log_q < -700.0) return 1.0;
        
        double q = safe_exp(log_q);
        return 1.0 - q;
    }
}

/**
 * @brief Chi-Square distribution PDF calculation
 * Formula: f(x) = (1/(2^(k/2) * Γ(k/2))) * x^(k/2-1) * exp(-x/2) for x ≥ 0
 */
double chi_square_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 1) {
        return NAN;
    }
    
    if (!chi_square_validate_params(params, param_count)) {
        return NAN;
    }
    
    double k = params[0]; // degrees of freedom
    
    if (!is_finite_number(x)) {
        if (x == INFINITY) return 0.0;
        if (x == -INFINITY) return 0.0;
        return NAN;
    }
    
    // PDF is 0 for negative x
    if (x < 0.0) {
        return 0.0;
    }
    
    // Special case for x = 0
    if (x == 0.0) {
        if (k < 2.0) return INFINITY;
        if (k == 2.0) return 0.5;
        return 0.0;
    }
    
    // Calculate PDF: (1/(2^(k/2) * Γ(k/2))) * x^(k/2-1) * exp(-x/2)
    double half_k = k / 2.0;
    double log_coefficient = -half_k * M_LN_2 - log_gamma_function(half_k);
    double log_power = (half_k - 1.0) * safe_log(x);
    double log_exp = -x / 2.0;
    
    return safe_exp(log_coefficient + log_power + log_exp);
}

/**
 * @brief Chi-Square distribution CDF calculation
 * Formula: F(x) = γ(k/2, x/2) / Γ(k/2) where γ is the lower incomplete gamma function
 */
double chi_square_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 1) {
        return NAN;
    }
    
    if (!chi_square_validate_params(params, param_count)) {
        return NAN;
    }
    
    double k = params[0]; // degrees of freedom
    
    if (!is_finite_number(x)) {
        if (x == -INFINITY) return 0.0;
        if (x == INFINITY) return 1.0;
        return NAN;
    }
    
    // CDF is 0 for negative x
    if (x <= 0.0) {
        return 0.0;
    }
    
    // Calculate CDF using regularized incomplete gamma function
    double half_k = k / 2.0;
    double half_x = x / 2.0;
    
    return regularized_incomplete_gamma_p(half_k, half_x);
}

/**
 * @brief Validate Chi-Square distribution parameters
 * Parameters: degrees_of_freedom (positive real number)
 */
int chi_square_validate_params(double* params, int param_count) {
    if (!params || param_count != 1) {
        return 0;
    }
    
    double k = params[0];
    
    // Degrees of freedom must be positive and finite
    if (!is_finite_number(k) || k <= 0.0) {
        return 0;
    }
    
    return 1;
}

/**
 * @brief Get the Chi-Square distribution interface
 */
const distribution_t* get_chi_square_distribution(void) {
    static const distribution_t chi_square_dist = {
        .pdf = chi_square_pdf,
        .cdf = chi_square_cdf,
        .validate_params = chi_square_validate_params,
        .distribution_name = "Chi-Square",
        .param_count = 1,
        .param_names = chi_square_param_names
    };
    
    return &chi_square_dist;
}