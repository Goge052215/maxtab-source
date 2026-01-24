#ifndef STATISTICAL_CONSTANTS_H
#define STATISTICAL_CONSTANTS_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// Mathematical constants with high precision
#define STAT_PI 3.14159265358979323846264338327950288
#define STAT_E 2.71828182845904523536028747135266250
#define STAT_SQRT_2PI 2.50662827463100050241576528481104525
#define STAT_SQRT_PI 1.77245385090551602729816748334114518
#define STAT_LN_2 0.69314718055994530941723212145817657
#define STAT_LN_10 2.30258509299404568401799145468436421
#define STAT_LOG10_E 0.43429448190325182765112891891660508
#define STAT_SQRT_2 1.41421356237309504880168872420969808
#define STAT_EULER_GAMMA 0.57721566490153286060651209008240243

// Small factorial cache for very fast lookup (0! to 12!)
#define SMALL_FACTORIAL_CACHE_SIZE 13

// Small factorial cache for performance optimization
extern const double small_factorial_cache[SMALL_FACTORIAL_CACHE_SIZE];

// Core statistical calculation functions using approximation algorithms
double calculate_factorial(int n);
double calculate_log_factorial(int n);
double calculate_normal_cdf(double z);
double calculate_chi_square_critical(int df, double alpha);
double calculate_t_critical(int df, double alpha);
double calculate_f_critical(int df1, int df2, double alpha);

// High-performance approximation algorithms for statistical functions
double stirling_log_factorial_approximation(int n);
double hart_normal_cdf_approximation(double z);
double wilson_hilferty_chi_square_approximation(int df, double alpha);
double cornish_fisher_t_approximation(int df, double alpha);
double beta_incomplete_f_approximation(int df1, int df2, double alpha);

// Advanced mathematical utility functions
double lanczos_gamma_approximation(double z);
double continued_fraction_gamma(double a, double x);
double rational_chebyshev_approximation(double x, const double* coeffs, int n);
double inverse_normal_cdf_beasley_springer(double p);
double error_function_abramowitz_stegun(double x);

// Utility functions for efficient computation
double fast_exp_approximation(double x);
double fast_log_approximation(double x);
double fast_sqrt_approximation(double x);
double fast_pow_approximation(double base, double exp);

// Performance benchmarking functions
typedef struct {
    double calculation_time_ns;
    double cpu_cycles;
    double accuracy_error;
    double max_error;
    int test_count;
    bool passed_accuracy_threshold;
} benchmark_result_t;

typedef struct {
    double total_time_ns;
    double avg_time_per_call_ns;
    double min_time_ns;
    double max_time_ns;
    double cpu_utilization_percent;
    int successful_calculations;
    int failed_calculations;
} performance_metrics_t;

// Comprehensive performance benchmarking
benchmark_result_t benchmark_factorial_approximation(int max_n, int iterations);
benchmark_result_t benchmark_normal_cdf_approximation(double z_min, double z_max, int iterations);
benchmark_result_t benchmark_critical_values_approximation(int max_df, int iterations);
benchmark_result_t benchmark_gamma_approximation(double min_val, double max_val, int iterations);

// Real-time performance monitoring
performance_metrics_t monitor_calculation_performance(void (*calc_function)(void), int iterations);
double measure_cpu_utilization_during_calculation(void (*calc_function)(void));
bool validate_performance_requirements(const performance_metrics_t* metrics);

// Memory usage optimization functions
void optimize_memory_usage(void);
size_t get_current_memory_usage(void);
void cleanup_calculation_cache(void);

#ifdef __cplusplus
}
#endif

#endif // STATISTICAL_CONSTANTS_H