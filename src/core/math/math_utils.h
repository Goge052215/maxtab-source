#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Mathematical constants
#define M_PI_PRECISE 3.14159265358979323846
#define M_E_PRECISE 2.71828182845904523536
#define M_SQRT_2PI 2.50662827463100050241
#define M_SQRT2 1.41421356237309504880
#define M_LN_2 0.69314718055994530942
#define M_LN_10 2.30258509299404568402

// Error codes
typedef enum {
    MATH_SUCCESS = 0,
    MATH_ERROR_INVALID_INPUT = -1,
    MATH_ERROR_OVERFLOW = -2,
    MATH_ERROR_UNDERFLOW = -3,
    MATH_ERROR_DOMAIN = -4
} math_error_t;

// Gamma function approximation using Lanczos approximation
double gamma_function(double x);
double log_gamma_function(double x);

// Factorial and combination functions
double factorial(int n);
double log_factorial(int n);
double combination(int n, int k);
double log_combination(int n, int k);

// Error function for normal distribution
double error_function(double x);
double complementary_error_function(double x);
double inverse_error_function(double x);

// Beta function
double beta_function(double a, double b);
double log_beta_function(double a, double b);

// Utility functions
bool is_finite_number(double x);
bool is_valid_probability(double p);
bool is_positive_integer(double x);
double safe_exp(double x);
double safe_log(double x);

#ifdef __cplusplus
}
#endif

#endif // MATH_UTILS_H