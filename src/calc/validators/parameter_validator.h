#ifndef PARAMETER_VALIDATOR_H
#define PARAMETER_VALIDATOR_H

#include "../../core/distributions/lib/distribution_interface.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum length for error messages
 */
#define MAX_ERROR_MESSAGE_LENGTH 128

/**
 * @brief Parameter validation error codes
 */
typedef enum {
    VALIDATION_SUCCESS = 0,
    VALIDATION_ERROR_INVALID_COUNT = 1,
    VALIDATION_ERROR_OUT_OF_RANGE = 2,
    VALIDATION_ERROR_INVALID_FORMAT = 3,
    VALIDATION_ERROR_MATHEMATICAL_CONSTRAINT = 4,
    VALIDATION_ERROR_NULL_POINTER = 5,
    VALIDATION_ERROR_UNKNOWN_DISTRIBUTION = 6
} validation_error_t;

/**
 * @brief Parameter validation result structure
 */
typedef struct {
    validation_error_t error_code;
    uint8_t invalid_parameter_index;
    char error_message[MAX_ERROR_MESSAGE_LENGTH];
    double suggested_value;  // Suggested value for out-of-range parameters
    int has_suggestion;      // Whether a suggestion is available
} validation_result_t;

/**
 * @brief Main parameter validation functions
 */
validation_result_t validate_distribution_parameters(distribution_type_t distribution, 
                                                   const double* parameters, 
                                                   uint8_t param_count);

validation_result_t validate_single_parameter(distribution_type_t distribution,
                                            uint8_t param_index,
                                            double value);

validation_result_t validate_parameter_count(distribution_type_t distribution,
                                           uint8_t provided_count);

/**
 * @brief Range validation functions
 */
int is_parameter_in_range(distribution_type_t distribution, 
                         uint8_t param_index, 
                         double value);

validation_result_t validate_parameter_range(distribution_type_t distribution,
                                           uint8_t param_index,
                                           double value);

/**
 * @brief Mathematical constraint validation functions
 */
validation_result_t validate_mathematical_constraints(distribution_type_t distribution,
                                                    const double* parameters,
                                                    uint8_t param_count);

/**
 * @brief Error message generation functions
 */
void generate_range_error_message(distribution_type_t distribution,
                                uint8_t param_index,
                                double value,
                                char* message,
                                size_t message_size);

void generate_constraint_error_message(distribution_type_t distribution,
                                     const char* constraint_description,
                                     char* message,
                                     size_t message_size);

void generate_format_error_message(const char* input_string,
                                 char* message,
                                 size_t message_size);

/**
 * @brief Parameter suggestion functions
 */
double suggest_parameter_value(distribution_type_t distribution,
                             uint8_t param_index,
                             double invalid_value);

int has_parameter_suggestion(distribution_type_t distribution,
                           uint8_t param_index);

/**
 * @brief Utility functions
 */
const char* get_validation_error_description(validation_error_t error_code);
void clear_validation_result(validation_result_t* result);
int is_finite_number(double value);
int is_positive_number(double value);
int is_probability_value(double value);

#ifdef __cplusplus
}
#endif

#endif // PARAMETER_VALIDATOR_H