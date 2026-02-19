#ifndef CALCULATION_ORCHESTRATOR_H
#define CALCULATION_ORCHESTRATOR_H

#include <stdint.h>
#include "../../models/state/app_state.h"
#include "../../core/distributions/lib/distribution_interface.h"

/**
 * @brief Calculation result structure
 */
typedef struct {
    double pdf_result;
    double cdf_result;
    double input_value;
    int success;
    const char* error_message;
} calculation_result_t;

/**
 * @brief Calculation request structure
 */
typedef struct {
    distribution_type_t distribution;
    double parameters[MAX_PARAMETERS];
    uint8_t param_count;
    double input_value;
} calculation_request_t;

/**
 * @brief Orchestrator error codes
 */
typedef enum {
    CALC_SUCCESS = 0,
    CALC_ERROR_INVALID_DISTRIBUTION = -1,
    CALC_ERROR_INVALID_PARAMETERS = -2,
    CALC_ERROR_INVALID_INPUT = -3,
    CALC_ERROR_CALCULATION_FAILED = -4,
    CALC_ERROR_STATE_INVALID = -5
} calculation_error_t;

/**
 * @brief Main calculation orchestration functions
 */
int orchestrator_calculate(app_state_t* state, double input_value, calculation_result_t* result);
int orchestrator_calculate_with_request(const calculation_request_t* request, calculation_result_t* result);
int orchestrator_validate_calculation_request(const calculation_request_t* request);

/**
 * @brief Input processing functions
 */
int orchestrator_process_input(const char* input_str, double* value);
int orchestrator_validate_input_value(double input_value, distribution_type_t distribution);

/**
 * @brief Result formatting functions
 */
void orchestrator_format_result(const calculation_result_t* result, char* buffer, size_t buffer_size);
void orchestrator_format_scientific(double value, char* buffer, size_t buffer_size);
int orchestrator_should_use_scientific(double value);

/**
 * @brief Error handling functions
 */
const char* orchestrator_get_error_message(calculation_error_t error);
void orchestrator_handle_calculation_error(calculation_error_t error, const char** user_message);

/**
 * @brief State management integration
 */
int orchestrator_update_state_with_result(app_state_t* state, const calculation_result_t* result);
int orchestrator_prepare_calculation_from_state(const app_state_t* state, calculation_request_t* request);

#endif // CALCULATION_ORCHESTRATOR_H