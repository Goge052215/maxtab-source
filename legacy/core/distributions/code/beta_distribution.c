#include "beta_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

static const char* beta_param_names[] = {"alpha", "beta"};

double beta_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!beta_validate_params(params, param_count)) {
        return NAN;
    }
    
    double alpha = params[0];
    double beta_param = params[1];
    
    if (!is_finite_number(x) || x < 0 || x > 1) {
        return 0.0;
    }
    
    if ((x == 0 && alpha < 1) || (x == 1 && beta_param < 1)) {
        return INFINITY;
    }
    
    if ((x == 0 && alpha > 1) || (x == 1 && beta_param > 1)) {
        return 0.0;
    }
    
    if ((x == 0 && alpha == 1) || (x == 1 && beta_param == 1)) {
        return beta_param; // or alpha, depends on which end
    }
    
    double log_beta_func = log_beta(alpha, beta_param);
    double term1 = (alpha - 1) * log(x);
    double term2 = (beta_param - 1) * log(1 - x);
    
    return safe_exp(term1 + term2 - log_beta_func);
}

double beta_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!beta_validate_params(params, param_count)) {
        return NAN;
    }
    
    double alpha = params[0];
    double beta_param = params[1];
    
    if (!is_finite_number(x)) {
        if (x == -INFINITY) return 0.0;
        if (x == INFINITY) return 1.0;
        return NAN;
    }
    
    if (x <= 0) {
        return 0.0;
    }
    
    if (x >= 1) {
        return 1.0;
    }
    
    return regularized_incomplete_beta(x, alpha, beta_param);
}

int beta_validate_params(double* params, int param_count) {
    if (!params || param_count != 2) {
        return 0;
    }
    
    double alpha = params[0];
    double beta_param = params[1];
    
    if (!is_finite_number(alpha) || alpha <= 0) {
        return 0;
    }
    
    if (!is_finite_number(beta_param) || beta_param <= 0) {
        return 0;
    }
    
    return 1;
}

const distribution_t* get_beta_distribution(void) {
    static const distribution_t beta_dist = {
        .pdf = beta_pdf,
        .cdf = beta_cdf,
        .validate_params = beta_validate_params,
        .distribution_name = "Beta",
        .param_count = 2,
        .param_names = beta_param_names
    };
    
    return &beta_dist;
}