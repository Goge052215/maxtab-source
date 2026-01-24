#include "hypergeometric_distribution.h"
#include "../math/math_utils.h"
#include <math.h>
#include <stddef.h>

// Parameter names for Hypergeometric distribution
static const char* hypergeometric_param_names[] = {"N", "K", "n"};

// Forward declarations for helper functions
static int min_int(int a, int b);
static int max_int(int a, int b);

/**
 * @brief Hypergeometric distribution PDF calculation
 * Formula: P(X = k) = C(K,k) * C(N-K,n-k) / C(N,n)
 * where C(n,k) is the binomial coefficient "n choose k"
 */
double hypergeometric_pdf(double x, double* params, int param_count) {
    if (!params || param_count != 3) {
        return NAN;
    }
    
    if (!hypergeometric_validate_params(params, param_count)) {
        return NAN;
    }
    
    int N = (int)params[0]; // population size
    int K = (int)params[1]; // number of success states in population
    int n = (int)params[2]; // sample size
    
    if (!is_finite_number(x)) {
        return NAN;
    }
    
    // Check if x is a non-negative integer
    if (x < 0.0 || floor(x) != x) {
        return 0.0;
    }
    
    int k = (int)x; // number of successes in sample
    
    // Check if k is within valid range
    int k_min = max_int(0, n - (N - K));
    int k_max = min_int(n, K);
    
    if (k < k_min || k > k_max) {
        return 0.0;
    }
    
    // Calculate PDF using log space for numerical stability
    // log(P(X = k)) = log(C(K,k)) + log(C(N-K,n-k)) - log(C(N,n))
    double log_prob = log_combination(K, k) + 
                      log_combination(N - K, n - k) - 
                      log_combination(N, n);
    
    return safe_exp(log_prob);
}

/**
 * @brief Hypergeometric distribution CDF calculation
 * Formula: P(X ≤ k) = sum_{i=0}^{k} P(X = i)
 */
double hypergeometric_cdf(double x, double* params, int param_count) {
    if (!params || param_count != 3) {
        return NAN;
    }
    
    if (!hypergeometric_validate_params(params, param_count)) {
        return NAN;
    }
    
    int N = (int)params[0]; // population size
    int K = (int)params[1]; // number of success states in population
    int n = (int)params[2]; // sample size
    
    if (!is_finite_number(x)) {
        if (x == -INFINITY) return 0.0;
        if (x == INFINITY) return 1.0;
        return NAN;
    }
    
    // For discrete distribution, use floor of x
    int k = (int)floor(x);
    
    // Determine valid range
    int k_min = max_int(0, n - (N - K));
    int k_max = min_int(n, K);
    
    // CDF is 0 for k < k_min
    if (k < k_min) {
        return 0.0;
    }
    
    // CDF is 1 for k >= k_max
    if (k >= k_max) {
        return 1.0;
    }
    
    // Sum PDF values from k_min to k
    double cdf = 0.0;
    for (int i = k_min; i <= k; i++) {
        cdf += hypergeometric_pdf((double)i, params, param_count);
    }
    
    return cdf;
}

/**
 * @brief Validate Hypergeometric distribution parameters
 * Parameters: N (population size), K (success states), n (sample size)
 * Constraints: N >= 1, 0 <= K <= N, 0 <= n <= N
 */
int hypergeometric_validate_params(double* params, int param_count) {
    if (!params || param_count != 3) {
        return 0;
    }
    
    double N_double = params[0];
    double K_double = params[1];
    double n_double = params[2];
    
    // All parameters must be finite
    if (!is_finite_number(N_double) || !is_finite_number(K_double) || !is_finite_number(n_double)) {
        return 0;
    }
    
    // All parameters must be non-negative integers
    if (N_double < 0.0 || floor(N_double) != N_double ||
        K_double < 0.0 || floor(K_double) != K_double ||
        n_double < 0.0 || floor(n_double) != n_double) {
        return 0;
    }
    
    int N = (int)N_double;
    int K = (int)K_double;
    int n = (int)n_double;
    
    // Population size must be at least 1
    if (N < 1) {
        return 0;
    }
    
    // Number of success states must not exceed population size
    if (K > N) {
        return 0;
    }
    
    // Sample size must not exceed population size
    if (n > N) {
        return 0;
    }
    
    return 1;
}

/**
 * @brief Get the Hypergeometric distribution interface
 */
const distribution_t* get_hypergeometric_distribution(void) {
    static const distribution_t hypergeometric_dist = {
        .pdf = hypergeometric_pdf,
        .cdf = hypergeometric_cdf,
        .validate_params = hypergeometric_validate_params,
        .distribution_name = "Hypergeometric",
        .param_count = 3,
        .param_names = hypergeometric_param_names
    };
    
    return &hypergeometric_dist;
}



/**
 * @brief Helper function to find minimum of two integers
 */
static int min_int(int a, int b) {
    return (a < b) ? a : b;
}

/**
 * @brief Helper function to find maximum of two integers
 */
static int max_int(int a, int b) {
    return (a > b) ? a : b;
}