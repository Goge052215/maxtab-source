#include "uniform_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

static const char* uniform_param_names[] = {"a", "b"};

double uniform_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!uniform_validate_params(params, param_count)) {
        return NAN;
    }
    
    double a = params[0];
    double b = params[1];
    
    if (!is_finite_number(x)) {
        return NAN;
    }
    
    if (x >= a && x <= b) {
        return 1.0 / (b - a);
    }
    
    return 0.0;
}

double uniform_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 2) {
        return NAN;
    }
    
    if (!uniform_validate_params(params, param_count)) {
        return NAN;
    }
    
    double a = params[0];
    double b = params[1];
    
    if (!is_finite_number(x)) {
        if (x == -INFINITY) return 0.0;
        if (x == INFINITY) return 1.0;
        return NAN;
    }
    
    if (x < a) {
        return 0.0;
    }
    
    if (x >= b) {
        return 1.0;
    }
    
    return (x - a) / (b - a);
}

int uniform_validate_params(double* params, int param_count) {
    if (!params || param_count != 2) {
        return 0;
    }
    
    double a = params[0];
    double b = params[1];
    
    if (!is_finite_number(a) || !is_finite_number(b)) {
        return 0;
    }
    
    if (a >= b) {
        return 0;
    }
    
    return 1;
}

const distribution_t* get_uniform_distribution(void) {
    static const distribution_t uniform_dist = {
        .pdf = uniform_pdf,
        .cdf = uniform_cdf,
        .validate_params = uniform_validate_params,
        .distribution_name = "Uniform",
        .param_count = 2,
        .param_names = uniform_param_names
    };
    
    return &uniform_dist;
}