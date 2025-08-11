#include "parameter_validator.h"
#include "../../models/distributions/distribution_registry.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>

/**
 * @brief Clear validation result structure
 */
void clear_validation_result(validation_result_t* result) {
    if (!result) return;
    
    result->error_code = VALIDATION_SUCCESS;
    result->invalid_parameter_index = 0;
    memset(result->error_message, 0, MAX_ERROR_MESSAGE_LENGTH);
    result->suggested_value = 0.0;
    result->has_suggestion = 0;
}

/**
 * @brief Check if a number is finite (not NaN or infinite)
 */
int is_finite_number(double value) {
    return isfinite(value);
}

/**
 * @brief Check if a number is positive
 */
int is_positive_number(double value) {
    return is_finite_number(value) && value > 0.0;
}

/**
 * @brief Check if a value is a valid probability (0 < p < 1)
 */
int is_probability_value(double value) {
    return is_finite_number(value) && value > 0.0 && value < 1.0;
}

/**
 * @brief Validate parameter count for a distribution
 */
validation_result_t validate_parameter_count(distribution_type_t distribution, uint8_t provided_count) {
    validation_result_t result;
    clear_validation_result(&result);
    
    if (!registry_is_valid_distribution_type(distribution)) {
        result.error_code = VALIDATION_ERROR_UNKNOWN_DISTRIBUTION;
        snprintf(result.error_message, MAX_ERROR_MESSAGE_LENGTH,
                "Unknown distribution type: %d", distribution);
        return result;
    }
    
    uint8_t expected_count = registry_get_parameter_count(distribution);
    if (provided_count != expected_count) {
        result.error_code = VALIDATION_ERROR_INVALID_COUNT;
        const char* dist_name = registry_get_distribution_name(distribution);
        snprintf(result.error_message, MAX_ERROR_MESSAGE_LENGTH,
                "%s distribution requires %d parameters, but %d provided",
                dist_name, expected_count, provided_count);
    }
    
    return result;
}

/**
 * @brief Check if parameter is within valid range
 */
int is_parameter_in_range(distribution_type_t distribution, uint8_t param_index, double value) {
    const double* range = registry_get_parameter_ranges(distribution, param_index);
    if (!range) return 0;
    
    return (value >= range[0] && value <= range[1]);
}

/**
 * @brief Validate parameter range
 */
validation_result_t validate_parameter_range(distribution_type_t distribution,
                                           uint8_t param_index,
                                           double value) {
    validation_result_t result;
    clear_validation_result(&result);
    
    if (!is_finite_number(value)) {
        result.error_code = VALIDATION_ERROR_INVALID_FORMAT;
        snprintf(result.error_message, MAX_ERROR_MESSAGE_LENGTH,
                "Parameter value must be a finite number");
        return result;
    }
    
    if (!is_parameter_in_range(distribution, param_index, value)) {
        result.error_code = VALIDATION_ERROR_OUT_OF_RANGE;
        result.invalid_parameter_index = param_index;
        generate_range_error_message(distribution, param_index, value,
                                   result.error_message, MAX_ERROR_MESSAGE_LENGTH);
        
        // Generate suggestion
        result.suggested_value = suggest_parameter_value(distribution, param_index, value);
        result.has_suggestion = has_parameter_suggestion(distribution, param_index);
    }
    
    return result;
}

/**
 * @brief Validate mathematical constraints specific to distributions
 */
validation_result_t validate_mathematical_constraints(distribution_type_t distribution,
                                                    const double* parameters,
                                                    uint8_t param_count) {
    validation_result_t result;
    clear_validation_result(&result);
    
    switch (distribution) {
        case DIST_HYPERGEOMETRIC:
            // For hypergeometric: success_states <= population_size, sample_size <= population_size
            if (param_count >= 3) {
                double population = parameters[0];
                double success_states = parameters[1];
                double sample_size = parameters[2];
                
                if (success_states > population) {
                    result.error_code = VALIDATION_ERROR_MATHEMATICAL_CONSTRAINT;
                    generate_constraint_error_message(distribution,
                        "Success states cannot exceed population size",
                        result.error_message, MAX_ERROR_MESSAGE_LENGTH);
                    result.invalid_parameter_index = 1;
                    result.suggested_value = population;
                    result.has_suggestion = 1;
                } else if (sample_size > population) {
                    result.error_code = VALIDATION_ERROR_MATHEMATICAL_CONSTRAINT;
                    generate_constraint_error_message(distribution,
                        "Sample size cannot exceed population size",
                        result.error_message, MAX_ERROR_MESSAGE_LENGTH);
                    result.invalid_parameter_index = 2;
                    result.suggested_value = population;
                    result.has_suggestion = 1;
                }
            }
            break;
            
        case DIST_F_DISTRIBUTION:
            // F-distribution degrees of freedom should be positive integers
            if (param_count >= 2) {
                double df1 = parameters[0];
                double df2 = parameters[1];
                
                if (df1 < 1.0 || df2 < 1.0) {
                    result.error_code = VALIDATION_ERROR_MATHEMATICAL_CONSTRAINT;
                    generate_constraint_error_message(distribution,
                        "Degrees of freedom must be at least 1",
                        result.error_message, MAX_ERROR_MESSAGE_LENGTH);
                    result.invalid_parameter_index = (df1 < 1.0) ? 0 : 1;
                    result.suggested_value = 1.0;
                    result.has_suggestion = 1;
                }
            }
            break;
            
        case DIST_BINOMIAL:
        case DIST_NEGATIVE_BINOMIAL:
            // For binomial and negative binomial, n should be a positive integer
            if (param_count >= 1) {
                double n = parameters[0];
                if (n < 1.0 || floor(n) != n) {
                    result.error_code = VALIDATION_ERROR_MATHEMATICAL_CONSTRAINT;
                    generate_constraint_error_message(distribution,
                        "Number of trials must be a positive integer",
                        result.error_message, MAX_ERROR_MESSAGE_LENGTH);
                    result.invalid_parameter_index = 0;
                    result.suggested_value = round(fmax(1.0, n));
                    result.has_suggestion = 1;
                }
            }
            break;
            
        default:
            // No additional constraints for other distributions
            break;
    }
    
    return result;
}

/**
 * @brief Validate single parameter
 */
validation_result_t validate_single_parameter(distribution_type_t distribution,
                                            uint8_t param_index,
                                            double value) {
    validation_result_t result;
    clear_validation_result(&result);
    
    if (!registry_is_valid_distribution_type(distribution)) {
        result.error_code = VALIDATION_ERROR_UNKNOWN_DISTRIBUTION;
        snprintf(result.error_message, MAX_ERROR_MESSAGE_LENGTH,
                "Unknown distribution type: %d", distribution);
        return result;
    }
    
    uint8_t param_count = registry_get_parameter_count(distribution);
    if (param_index >= param_count) {
        result.error_code = VALIDATION_ERROR_INVALID_COUNT;
        snprintf(result.error_message, MAX_ERROR_MESSAGE_LENGTH,
                "Parameter index %d is invalid for distribution with %d parameters",
                param_index, param_count);
        return result;
    }
    
    return validate_parameter_range(distribution, param_index, value);
}

/**
 * @brief Main parameter validation function
 */
validation_result_t validate_distribution_parameters(distribution_type_t distribution,
                                                   const double* parameters,
                                                   uint8_t param_count) {
    validation_result_t result;
    clear_validation_result(&result);
    
    if (!parameters) {
        result.error_code = VALIDATION_ERROR_NULL_POINTER;
        snprintf(result.error_message, MAX_ERROR_MESSAGE_LENGTH,
                "Parameters array cannot be null");
        return result;
    }
    
    // Validate parameter count
    result = validate_parameter_count(distribution, param_count);
    if (result.error_code != VALIDATION_SUCCESS) {
        return result;
    }
    
    // Validate each parameter range
    for (uint8_t i = 0; i < param_count; i++) {
        result = validate_parameter_range(distribution, i, parameters[i]);
        if (result.error_code != VALIDATION_SUCCESS) {
            return result;
        }
    }
    
    // Validate mathematical constraints
    result = validate_mathematical_constraints(distribution, parameters, param_count);
    
    return result;
}

/**
 * @brief Generate range error message
 */
void generate_range_error_message(distribution_type_t distribution,
                                uint8_t param_index,
                                double value,
                                char* message,
                                size_t message_size) {
    if (!message) return;
    
    const char* dist_name = registry_get_distribution_name(distribution);
    const char** param_names = registry_get_parameter_names(distribution);
    const double* range = registry_get_parameter_ranges(distribution, param_index);
    
    if (!dist_name || !param_names || !range) {
        snprintf(message, message_size, "Parameter validation error");
        return;
    }
    
    const char* param_name = param_names[param_index];
    
    snprintf(message, message_size,
            "%s parameter '%s' (%.3f) must be between %.3f and %.3f",
            dist_name, param_name, value, range[0], range[1]);
}

/**
 * @brief Generate constraint error message
 */
void generate_constraint_error_message(distribution_type_t distribution,
                                     const char* constraint_description,
                                     char* message,
                                     size_t message_size) {
    if (!message || !constraint_description) return;
    
    const char* dist_name = registry_get_distribution_name(distribution);
    if (!dist_name) {
        snprintf(message, message_size, "Mathematical constraint error: %s", constraint_description);
    } else {
        snprintf(message, message_size, "%s: %s", dist_name, constraint_description);
    }
}

/**
 * @brief Generate format error message
 */
void generate_format_error_message(const char* input_string,
                                 char* message,
                                 size_t message_size) {
    if (!message) return;
    
    if (input_string) {
        snprintf(message, message_size,
                "Invalid number format: '%s'. Please enter a valid number.", input_string);
    } else {
        snprintf(message, message_size, "Invalid number format. Please enter a valid number.");
    }
}

/**
 * @brief Suggest parameter value for out-of-range inputs
 */
double suggest_parameter_value(distribution_type_t distribution,
                             uint8_t param_index,
                             double invalid_value) {
    const double* range = registry_get_parameter_ranges(distribution, param_index);
    if (!range) return invalid_value;
    
    // If value is below minimum, suggest minimum
    if (invalid_value < range[0]) {
        return range[0];
    }
    
    // If value is above maximum, suggest maximum
    if (invalid_value > range[1]) {
        return range[1];
    }
    
    // If somehow in range but still invalid, suggest midpoint
    return (range[0] + range[1]) / 2.0;
}

/**
 * @brief Check if parameter suggestion is available
 */
int has_parameter_suggestion(distribution_type_t distribution, uint8_t param_index) {
    const double* range = registry_get_parameter_ranges(distribution, param_index);
    return (range != NULL);
}

/**
 * @brief Get description of validation error
 */
const char* get_validation_error_description(validation_error_t error_code) {
    switch (error_code) {
        case VALIDATION_SUCCESS:
            return "Validation successful";
        case VALIDATION_ERROR_INVALID_COUNT:
            return "Invalid parameter count";
        case VALIDATION_ERROR_OUT_OF_RANGE:
            return "Parameter out of valid range";
        case VALIDATION_ERROR_INVALID_FORMAT:
            return "Invalid number format";
        case VALIDATION_ERROR_MATHEMATICAL_CONSTRAINT:
            return "Mathematical constraint violation";
        case VALIDATION_ERROR_NULL_POINTER:
            return "Null pointer error";
        case VALIDATION_ERROR_UNKNOWN_DISTRIBUTION:
            return "Unknown distribution type";
        default:
            return "Unknown validation error";
    }
}