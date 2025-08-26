#include "rayleigh_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

static const char* rayleigh_param_names[] = {"scale"};

double rayleigh_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 1) {
        return NAN;
    }
    
    if (!rayleigh_validate_params(params, param_count)) {
        return NAN;
    }
    
    double scale = params[0];
    
    if (!is_finite_number(x) || x < 0) {
        return 0.0;
    }
    
    double scale_sq = scale * scale;
    double term1 = log(x) - 2 * log(scale);
    double term2 = -(x * x) / (2 * scale_sq);
    
    return safe_exp(term1 + term2);
}

double rayleigh_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 1) {
        return NAN;
    }
    
    if (!rayleigh_validate_params(params, param_count)) {
        return NAN;
    }
    
    double scale = params[0];
    
    if (!is_finite_number(x)) {
        if (x == -INFINITY) return 0.0;
        if (x == INFINITY) return 1.0;
        return NAN;
    }
    
    if (x < 0) {
        return 0.0;
    }
    
    double scale_sq = scale * scale;
    return 1.0 - safe_exp(-(x * x) / (2 * scale_sq));
}

int rayleigh_validate_params(double* params, int param_count) {
    if (!params || param_count != 1) {
        return 0;
    }
    
    double scale = params[0];
    
    if (!is_finite_number(scale) || scale <= 0) {
        return 0;
    }
    
    return 1;
}

const distribution_t* get_rayleigh_distribution(void) {
    static const distribution_t rayleigh_dist = {
        .pdf = rayleigh_pdf,
        .cdf = rayleigh_cdf,
        .validate_params = rayleigh_validate_params,
        .distribution_name = "Rayleigh",
        .param_count = 1,
        .param_names = rayleigh_param_names
    };
    
    return &rayleigh_dist;
}