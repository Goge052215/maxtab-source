#include "gamma_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

static const char* gamma_param_names[] = {"shape", "scale"};

double gamma_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!gamma_validate_params(params, param_count)) {
        return NAN;
    }
    
    double shape = params[0];
    double scale = params[1];
    
    if (!is_finite_number(x) || x < 0) {
        return 0.0;
    }
    
    if (x == 0 && shape == 1) {
        return 1.0 / scale;
    }
    
    if (x == 0 && shape > 1) {
        return 0.0;
    }
    
    if (x == 0 && shape < 1) {
        return INFINITY;
    }
    
    double term1 = (shape - 1) * log(x);
    double term2 = -x / scale;
    double term3 = -shape * log(scale);
    double term4 = -log_gamma(shape);
    
    return safe_exp(term1 + term2 + term3 + term4);
}

double gamma_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!gamma_validate_params(params, param_count)) {
        return NAN;
    }
    
    double shape = params[0];
    double scale = params[1];
    
    if (!is_finite_number(x)) {
        if (x == -INFINITY) return 0.0;
        if (x == INFINITY) return 1.0;
        return NAN;
    }
    
    if (x < 0) {
        return 0.0;
    }
    
    return lower_incomplete_gamma(shape, x / scale);
}

int gamma_validate_params(double* params, int param_count) {
    if (!params || param_count != 2) {
        return 0;
    }
    
    double shape = params[0];
    double scale = params[1];
    
    if (!is_finite_number(shape) || shape <= 0) {
        return 0;
    }
    
    if (!is_finite_number(scale) || scale <= 0) {
        return 0;
    }
    
    return 1;
}

const distribution_t* get_gamma_distribution(void) {
    static const distribution_t gamma_dist = {
        .pdf = gamma_pdf,
        .cdf = gamma_cdf,
        .validate_params = gamma_validate_params,
        .distribution_name = "Gamma",
        .param_count = 2,
        .param_names = gamma_param_names
    };
    
    return &gamma_dist;
}