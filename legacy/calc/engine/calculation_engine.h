#ifndef CALCULATION_ENGINE_H
#define CALCULATION_ENGINE_H

#include <stdint.h>
#include "../../core/distributions/lib/distribution_interface.h"

/**
 * @brief Calculation result structure
 */
typedef struct {
    double pdf_value;
    double cdf_value;
    int success;
    const char* error_message;
} calculation_result_t;

/**
 * @brief Error codes for calculation engine
 */
typedef enum {
    CALC_SUCCESS = 0,
    CALC_ERROR_INVALID_DISTRIBUTION = 1,
    CALC_ERROR_INVALID_PARAMETERS = 2,
    CALC_ERROR_MATH_ERROR = 3,
    CALC_ERROR_OUT_OF_MEMORY = 4
} calculation_error_t;

/**
 * @brief Calculation engine interface functions
 */
calculation_result_t calculate_distribution(
    distribution_type_t distribution,
    double input_value,
    const double* parameters,
    int param_count
);

int validate_distribution_parameters(
    distribution_type_t distribution,
    const double* parameters,
    int param_count
);

const char* get_calculation_error_message(calculation_error_t error);

#endif // CALCULATION_ENGINE_H