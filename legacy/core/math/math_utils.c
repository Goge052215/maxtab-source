#include "math_utils.h"
#include <math.h>
#include <float.h>

// Lanczos coefficients for gamma function approximation
static const double lanczos_coefficients[] = {
    0.99999999999980993,
    676.5203681218851,
    -1259.1392167224028,
    771.32342877765313,
    -176.61502916214059,
    12.507343278686905,
    -0.13857109526572012,
    9.9843695780195716e-6,
    1.5056327351493116e-7
};

static const int lanczos_g = 7;
static const double lanczos_sqrt_2pi = 2.5066282746310005024;

/**
 * Gamma function approximation using Lanczos approximation
 * Accurate to about 15 decimal places
 */
double gamma_function(double x) {
    if (x < 0.5) {
        // Use reflection formula: Γ(z)Γ(1-z) = π/sin(πz)
        return M_PI_PRECISE / (sin(M_PI_PRECISE * x) * gamma_function(1.0 - x));
    }
    
    x -= 1.0;
    double a = lanczos_coefficients[0];
    
    for (int i = 1; i < 9; i++) {
        a += lanczos_coefficients[i] / (x + i);
    }
    
    double t = x + lanczos_g + 0.5;
    return lanczos_sqrt_2pi * pow(t, x + 0.5) * exp(-t) * a;
}

/**
 * Log gamma function for numerical stability with large values
 */
double log_gamma_function(double x) {
    if (x < 0.5) {
        // Use reflection formula in log space
        return log(M_PI_PRECISE) - log(sin(M_PI_PRECISE * x)) - log_gamma_function(1.0 - x);
    }
    
    x -= 1.0;
    double a = lanczos_coefficients[0];
    
    for (int i = 1; i < 9; i++) {
        a += lanczos_coefficients[i] / (x + i);
    }
    
    double t = x + lanczos_g + 0.5;
    return log(lanczos_sqrt_2pi) + (x + 0.5) * log(t) - t + log(a);
}

/**
 * Factorial function with overflow protection
 */
double factorial(int n) {
    if (n < 0) return NAN;
    if (n == 0 || n == 1) return 1.0;
    if (n > 170) return INFINITY; // Overflow protection
    
    return gamma_function(n + 1.0);
}

/**
 * Log factorial for numerical stability
 */
double log_factorial(int n) {
    if (n < 0) return NAN;
    if (n == 0 || n == 1) return 0.0;
    
    return log_gamma_function(n + 1.0);
}

/**
 * Combination function C(n,k) = n! / (k! * (n-k)!)
 */
double combination(int n, int k) {
    if (k < 0 || k > n || n < 0) return 0.0;
    if (k == 0 || k == n) return 1.0;
    
    // Use symmetry: C(n,k) = C(n,n-k)
    if (k > n - k) k = n - k;
    
    // Use log space to avoid overflow
    return exp(log_combination(n, k));
}

/**
 * Log combination for numerical stability
 */
double log_combination(int n, int k) {
    if (k < 0 || k > n || n < 0) return -INFINITY;
    if (k == 0 || k == n) return 0.0;
    
    // Use symmetry: C(n,k) = C(n,n-k)
    if (k > n - k) k = n - k;
    
    return log_factorial(n) - log_factorial(k) - log_factorial(n - k);
}

/**
 * Error function using rational approximation
 * Accurate to about 1.5e-7
 */
double error_function(double x) {
    if (x == 0.0) return 0.0;
    
    // Constants for rational approximation
    static const double a1 =  0.254829592;
    static const double a2 = -0.284496736;
    static const double a3 =  1.421413741;
    static const double a4 = -1.453152027;
    static const double a5 =  1.061405429;
    static const double p  =  0.3275911;
    
    // Save the sign of x
    int sign = (x >= 0) ? 1 : -1;
    x = fabs(x);
    
    // A&S formula 7.1.26
    double t = 1.0 / (1.0 + p * x);
    double y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * exp(-x * x);
    
    return sign * y;
}

/**
 * Complementary error function
 */
double complementary_error_function(double x) {
    return 1.0 - error_function(x);
}

/**
 * Inverse error function using Newton-Raphson method
 */
double inverse_error_function(double x) {
    if (fabs(x) >= 1.0) return NAN;
    if (x == 0.0) return 0.0;
    
    // Initial approximation
    double w = -log((1.0 - x) * (1.0 + x));
    double p;
    
    if (w < 5.0) {
        w = w - 2.5;
        p = 2.81022636e-08;
        p = 3.43273939e-07 + p * w;
        p = -3.5233877e-06 + p * w;
        p = -4.39150654e-06 + p * w;
        p = 0.00021858087 + p * w;
        p = -0.00125372503 + p * w;
        p = -0.00417768164 + p * w;
        p = 0.246640727 + p * w;
        p = 1.50140941 + p * w;
    } else {
        w = sqrt(w) - 3.0;
        p = -0.000200214257;
        p = 0.000100950558 + p * w;
        p = 0.00134934322 + p * w;
        p = -0.00367342844 + p * w;
        p = 0.00573950773 + p * w;
        p = -0.0076224613 + p * w;
        p = 0.00943887047 + p * w;
        p = 1.00167406 + p * w;
        p = 2.83297682 + p * w;
    }
    
    return x * p;
}

/**
 * Beta function B(a,b) = Γ(a)Γ(b)/Γ(a+b)
 */
double beta_function(double a, double b) {
    if (a <= 0.0 || b <= 0.0) return NAN;
    
    return exp(log_beta_function(a, b));
}

/**
 * Log beta function for numerical stability
 */
double log_beta_function(double a, double b) {
    if (a <= 0.0 || b <= 0.0) return NAN;
    
    return log_gamma_function(a) + log_gamma_function(b) - log_gamma_function(a + b);
}

/**
 * Check if a number is finite (not NaN or infinite)
 */
bool is_finite_number(double x) {
    return isfinite(x);
}

/**
 * Check if a value is a valid probability (0 <= p <= 1)
 */
bool is_valid_probability(double p) {
    return is_finite_number(p) && p >= 0.0 && p <= 1.0;
}

/**
 * Check if a value is a positive integer
 */
bool is_positive_integer(double x) {
    return is_finite_number(x) && x > 0.0 && floor(x) == x;
}

/**
 * Safe exponential function with overflow protection
 */
double safe_exp(double x) {
    if (x > 700.0) return INFINITY;  // Prevent overflow
    if (x < -700.0) return 0.0;      // Prevent underflow
    return exp(x);
}

/**
 * Safe logarithm function with domain checking
 */
double safe_log(double x) {
    if (x <= 0.0) return NAN;
    if (x < DBL_MIN) return -INFINITY;
    return log(x);
}