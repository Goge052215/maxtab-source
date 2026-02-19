#include "calculation_orchestrator.h"
#include "../validators/parameter_validator.h"
#include "../../core/distributions/lib/distribution_interface.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**
 * @brief Main calculation orchestration function using application state
 * @param state Application state containing distribution and parameters
 * @param input_value Input value for PDF/CDF calculation
 * @param result Pointer to store calculation results
 * @return 0 on success, negative error code on failure
 */
int orchestrator_calculate(app_state_t* state, double input_value, calculation_result_t* result) {
    if (!state || !result) {
        return CALC_ERROR_STATE_INVALID;
    }
    
    // Validate application state
    if (app_state_validate(state) != 0) {
        result->success = 0;
        result->error_message = "Invalid application state";
        return CALC_ERROR_STATE_INVALID;
    }
    
    // Prepare calculation request from state
    calculation_request_t request;
    int prep_result = orchestrator_prepare_calculation_from_state(state, &request);
    if (prep_result != 0) {
        result->success = 0;
        result->error_message = "Failed to prepare calculation request";
        return prep_result;
    }
    
    // Set input value
    request.input_value = input_value;
    
    // Perform calculation
    int calc_result = orchestrator_calculate_with_request(&request, result);
    
    // Update state with result if successful
    if (calc_result == CALC_SUCCESS && result->success) {
        orchestrator_update_state_with_result(state, result);
    }
    
    return calc_result;
}

/**
 * @brief Calculate PDF and CDF using a calculation request
 * @param request Calculation request with distribution and parameters
 * @param result Pointer to store calculation results
 * @return 0 on success, negative error code on failure
 */
int orchestrator_calculate_with_request(const calculation_request_t* request, calculation_result_t* result) {
    if (!request || !result) {
        return CALC_ERROR_STATE_INVALID;
    }
    
    // Initialize result
    memset(result, 0, sizeof(calculation_result_t));
    result->input_value = request->input_value;
    
    // Validate calculation request
    int validation_result = orchestrator_validate_calculation_request(request);
    if (validation_result != CALC_SUCCESS) {
        result->success = 0;
        result->error_message = orchestrator_get_error_message(validation_result);
        return validation_result;
    }
    
    // Get distribution implementation
    const distribution_t* dist = get_distribution(request->distribution);
    if (!dist) {
        result->success = 0;
        result->error_message = "Distribution implementation not found";
        return CALC_ERROR_INVALID_DISTRIBUTION;
    }
    
    // Validate parameters using distribution's validator
    if (dist->validate_params) {
        double* params = (double*)request->parameters;
        if (dist->validate_params(params, request->param_count) != 0) {
            result->success = 0;
            result->error_message = "Invalid parameters for distribution";
            return CALC_ERROR_INVALID_PARAMETERS;
        }
    }
    
    // Validate input value for this distribution
    if (orchestrator_validate_input_value(request->input_value, request->distribution) != 0) {
        result->success = 0;
        result->error_message = "Invalid input value for distribution";
        return CALC_ERROR_INVALID_INPUT;
    }
    
    // Perform PDF calculation
    if (dist->pdf) {
        double* params = (double*)request->parameters;
        result->pdf_result = dist->pdf(request->input_value, params, request->param_count);
        
        // Check for calculation errors (NaN, infinity)
        if (isnan(result->pdf_result) || isinf(result->pdf_result)) {
            result->success = 0;
            result->error_message = "PDF calculation failed";
            return CALC_ERROR_CALCULATION_FAILED;
        }
    }
    
    // Perform CDF calculation
    if (dist->cdf) {
        double* params = (double*)request->parameters;
        result->cdf_result = dist->cdf(request->input_value, params, request->param_count);
        
        // Check for calculation errors (NaN, infinity)
        if (isnan(result->cdf_result) || isinf(result->cdf_result)) {
            result->success = 0;
            result->error_message = "CDF calculation failed";
            return CALC_ERROR_CALCULATION_FAILED;
        }
    }
    
    result->success = 1;
    result->error_message = NULL;
    return CALC_SUCCESS;
}

/**
 * @brief Validate a calculation request
 * @param request Calculation request to validate
 * @return 0 if valid, negative error code if invalid
 */
int orchestrator_validate_calculation_request(const calculation_request_t* request) {
    if (!request) {
        return CALC_ERROR_STATE_INVALID;
    }
    
    // Validate distribution type
    if (!is_valid_distribution_type(request->distribution)) {
        return CALC_ERROR_INVALID_DISTRIBUTION;
    }
    
    // Get distribution model for parameter validation
    const distribution_model_t* model = get_distribution_model(request->distribution);
    if (!model) {
        return CALC_ERROR_INVALID_DISTRIBUTION;
    }
    
    // Validate parameter count
    if (request->param_count != model->param_count) {
        return CALC_ERROR_INVALID_PARAMETERS;
    }
    
    // Validate individual parameters using parameter validator
    for (uint8_t i = 0; i < request->param_count; i++) {
        validation_result_t validation = validate_single_parameter(request->distribution, i, request->parameters[i]);
        if (validation.error_code != VALIDATION_SUCCESS) {
            return CALC_ERROR_INVALID_PARAMETERS;
        }
    }
    
    return CALC_SUCCESS;
}

/**
 * @brief Process string input and convert to double
 * @param input_str Input string to process
 * @param value Pointer to store converted value
 * @return 0 on success, -1 on error
 */
int orchestrator_process_input(const char* input_str, double* value) {
    if (!input_str || !value) {
        return -1;
    }
    
    char* endptr;
    *value = strtod(input_str, &endptr);
    
    // Check if conversion was successful
    if (endptr == input_str || *endptr != '\0') {
        return -1;  // Invalid number format
    }
    
    // Check for overflow/underflow
    if (*value == HUGE_VAL || *value == -HUGE_VAL) {
        return -1;  // Number too large
    }
    
    return 0;
}

/**
 * @brief Validate input value for specific distribution
 * @param input_value Input value to validate
 * @param distribution Distribution type
 * @return 0 if valid, -1 if invalid
 */
int orchestrator_validate_input_value(double input_value, distribution_type_t distribution) {
    // Check for NaN or infinity
    if (isnan(input_value) || isinf(input_value)) {
        return -1;
    }
    
    // Get distribution model
    const distribution_model_t* model = get_distribution_model(distribution);
    if (!model) {
        return -1;
    }
    
    // Apply distribution-specific input validation
    if (model->category == DISTRIBUTION_DISCRETE) {
        // Discrete distributions typically require non-negative integer inputs
        if (input_value < 0 || floor(input_value) != input_value) {
            return -1;
        }
    }
    
    // Additional validation can be added here for specific distributions
    return 0;
}

/**
 * @brief Format calculation result for display
 * @param result Calculation result to format
 * @param buffer Buffer to store formatted string
 * @param buffer_size Size of the buffer
 */
void orchestrator_format_result(const calculation_result_t* result, char* buffer, size_t buffer_size) {
    if (!result || !buffer || buffer_size == 0) {
        return;
    }
    
    if (!result->success) {
        snprintf(buffer, buffer_size, "Error: %s", 
                result->error_message ? result->error_message : "Unknown error");
        return;
    }
    
    char pdf_str[32];
    char cdf_str[32];
    
    // Format PDF result
    if (orchestrator_should_use_scientific(result->pdf_result)) {
        orchestrator_format_scientific(result->pdf_result, pdf_str, sizeof(pdf_str));
    } else {
        snprintf(pdf_str, sizeof(pdf_str), "%.4f", result->pdf_result);
    }
    
    // Format CDF result
    if (orchestrator_should_use_scientific(result->cdf_result)) {
        orchestrator_format_scientific(result->cdf_result, cdf_str, sizeof(cdf_str));
    } else {
        snprintf(cdf_str, sizeof(cdf_str), "%.4f", result->cdf_result);
    }
    
    snprintf(buffer, buffer_size, "PDF: %s\\nCDF: %s", pdf_str, cdf_str);
}

/**
 * @brief Format value in scientific notation
 * @param value Value to format
 * @param buffer Buffer to store formatted string
 * @param buffer_size Size of the buffer
 */
void orchestrator_format_scientific(double value, char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return;
    }
    
    snprintf(buffer, buffer_size, "%.2e", value);
}

/**
 * @brief Check if value should be displayed in scientific notation
 * @param value Value to check
 * @return 1 if should use scientific notation, 0 otherwise
 */
int orchestrator_should_use_scientific(double value) {
    double abs_value = fabs(value);
    return (abs_value < 0.0001 && abs_value > 0.0) || abs_value >= 10000.0;
}

/**
 * @brief Get error message for error code
 * @param error Error code
 * @return Error message string
 */
const char* orchestrator_get_error_message(calculation_error_t error) {
    switch (error) {
        case CALC_SUCCESS:
            return "Success";
        case CALC_ERROR_INVALID_DISTRIBUTION:
            return "Invalid distribution type";
        case CALC_ERROR_INVALID_PARAMETERS:
            return "Invalid parameters";
        case CALC_ERROR_INVALID_INPUT:
            return "Invalid input value";
        case CALC_ERROR_CALCULATION_FAILED:
            return "Calculation failed";
        case CALC_ERROR_STATE_INVALID:
            return "Invalid state";
        default:
            return "Unknown error";
    }
}

/**
 * @brief Handle calculation error and provide user-friendly message
 * @param error Error code
 * @param user_message Pointer to store user message
 */
void orchestrator_handle_calculation_error(calculation_error_t error, const char** user_message) {
    if (!user_message) {
        return;
    }
    
    switch (error) {
        case CALC_ERROR_INVALID_DISTRIBUTION:
            *user_message = "Please select a valid distribution";
            break;
        case CALC_ERROR_INVALID_PARAMETERS:
            *user_message = "Please check parameter values";
            break;
        case CALC_ERROR_INVALID_INPUT:
            *user_message = "Please enter a valid input value";
            break;
        case CALC_ERROR_CALCULATION_FAILED:
            *user_message = "Calculation not possible with these values";
            break;
        case CALC_ERROR_STATE_INVALID:
            *user_message = "Please restart the calculation";
            break;
        default:
            *user_message = "An error occurred. Please try again";
            break;
    }
}

/**
 * @brief Update application state with calculation result
 * @param state Application state to update
 * @param result Calculation result to add to history
 * @return 0 on success, -1 on error
 */
int orchestrator_update_state_with_result(app_state_t* state, const calculation_result_t* result) {
    if (!state || !result || !result->success) {
        return -1;
    }
    
    // Add calculation to history
    return app_state_add_calculation(state, result->input_value, 
                                   result->pdf_result, result->cdf_result);
}

/**
 * @brief Prepare calculation request from application state
 * @param state Application state
 * @param request Pointer to store prepared request
 * @return 0 on success, negative error code on failure
 */
int orchestrator_prepare_calculation_from_state(const app_state_t* state, calculation_request_t* request) {
    if (!state || !request) {
        return CALC_ERROR_STATE_INVALID;
    }
    
    // Validate state
    if (!state->is_initialized) {
        return CALC_ERROR_STATE_INVALID;
    }
    
    // Prepare request
    request->distribution = app_state_get_distribution(state);
    request->param_count = app_state_get_parameter_count(state);
    
    // Copy parameters
    const double* params = app_state_get_parameters(state);
    if (!params) {
        return CALC_ERROR_STATE_INVALID;
    }
    
    memcpy(request->parameters, params, sizeof(double) * request->param_count);
    
    return CALC_SUCCESS;
}