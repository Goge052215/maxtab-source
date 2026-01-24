#ifndef DISTRIBUTION_INTERFACE_H
#define DISTRIBUTION_INTERFACE_H

#include <stdint.h>

/**
 * @brief Maximum number of parameters for any distribution
 */
#define MAX_PARAMETERS 4

/**
 * @brief Distribution function interface for PDF and CDF calculations
 */
typedef struct {
    double (*pdf)(double x, double* params, int param_count);
    double (*cdf)(double x, double* params, int param_count);
    int (*validate_params)(double* params, int param_count);
    const char* distribution_name;
    int param_count;
    const char** param_names;
} distribution_t;

/**
 * @brief Distribution categories
 */
typedef enum {
    DISTRIBUTION_CONTINUOUS = 0,
    DISTRIBUTION_DISCRETE = 1
} distribution_category_t;

/**
 * @brief Distribution types enumeration
 */
typedef enum {
    // Continuous distributions
    DIST_NORMAL = 0,
    DIST_EXPONENTIAL = 1,
    DIST_CHI_SQUARE = 2,
    DIST_T_DISTRIBUTION = 3,
    DIST_F_DISTRIBUTION = 4,
    
    // Discrete distributions
    DIST_GEOMETRIC = 5,
    DIST_HYPERGEOMETRIC = 6,
    DIST_BINOMIAL = 7,
    DIST_NEGATIVE_BINOMIAL = 8,
    DIST_POISSON = 9,
    
    DIST_COUNT = 10
} distribution_type_t;

/**
 * @brief Distribution model structure
 */
typedef struct {
    uint8_t distribution_id;
    const char* name;
    uint8_t param_count;
    const char** param_names;
    double param_ranges[4][2];  // min/max for each parameter
    distribution_category_t category;
} distribution_model_t;

/**
 * @brief Function prototypes for distribution interface
 */
const distribution_t* get_distribution(distribution_type_t type);
const distribution_model_t* get_distribution_model(distribution_type_t type);
int is_valid_distribution_type(distribution_type_t type);

#endif // DISTRIBUTION_INTERFACE_H