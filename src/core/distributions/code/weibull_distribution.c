#include "weibull_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

static const char* weibull_param_names[] = {"shape", "scale"};

double weibull_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!weibull_validate_params(params, param_count)) {
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
    
    double term1 = log(shape) - log(scale);
    double term2 = (shape - 1) * (log(x) - log(scale));
    double term3 = -pow(x / scale, shape);
    
    return safe_exp(term1 + term2 + term3);
}

double weibull_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!weibull_validate_params(params, param_count)) {
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
    
    return 1.0 - safe_exp(-pow(x / scale, shape));
}

int weibull_validate_params(double* params, int param_count) {
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

const distribution_t* get_weibull_distribution(void) {
    static const distribution_t weibull_dist = {
        .pdf = weibull_pdf,
        .cdf = weibull_cdf,
        .validate_params = weibull_validate_params,
        .distribution_name = "Weibull",
        .param_count = 2,
        .param_names = weibull_param_names
    };
    
    return &weibull_dist;
}