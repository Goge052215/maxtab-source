#include "pareto_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

static const char* pareto_param_names[] = {"scale", "shape"};

double pareto_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!pareto_validate_params(params, param_count)) {
        return NAN;
    }
    
    double scale = params[0];
    double shape = params[1];
    
    if (!is_finite_number(x) || x < scale) {
        return 0.0;
    }
    
    double term1 = log(shape) + shape * log(scale);
    double term2 = -(shape + 1) * log(x);
    
    return safe_exp(term1 + term2);
}

double pareto_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!pareto_validate_params(params, param_count)) {
        return NAN;
    }
    
    double scale = params[0];
    double shape = params[1];
    
    if (!is_finite_number(x)) {
        if (x == -INFINITY) return 0.0;
        if (x == INFINITY) return 1.0;
        return NAN;
    }
    
    if (x < scale) {
        return 0.0;
    }
    
    return 1.0 - pow(scale / x, shape);
}

int pareto_validate_params(double* params, int param_count) {
    if (!params || param_count != 2) {
        return 0;
    }
    
    double scale = params[0];
    double shape = params[1];
    
    if (!is_finite_number(scale) || scale <= 0) {
        return 0;
    }
    
    if (!is_finite_number(shape) || shape <= 0) {
        return 0;
    }
    
    return 1;
}

const distribution_t* get_pareto_distribution(void) {
    static const distribution_t pareto_dist = {
        .pdf = pareto_pdf,
        .cdf = pareto_cdf,
        .validate_params = pareto_validate_params,
        .distribution_name = "Pareto",
        .param_count = 2,
        .param_names = pareto_param_names
    };
    
    return &pareto_dist;
}