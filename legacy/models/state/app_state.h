#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdint.h>
#include "../history/calculation_history.h"
#include "../../core/distributions/lib/distribution_interface.h"

/**
 * @brief Application state structure
 */
typedef struct {
    uint8_t current_distribution;
    distribution_category_t current_category;
    double current_parameters[MAX_PARAMETERS];
    uint8_t parameter_count;
    calculation_history_t history;
    uint8_t is_initialized;
} app_state_t;

/**
 * @brief Application state management functions
 */
void app_state_init(app_state_t* state);
int app_state_set_distribution(app_state_t* state, distribution_type_t distribution);
int app_state_set_parameter(app_state_t* state, uint8_t param_index, double value);
int app_state_add_calculation(app_state_t* state, double input_value, double pdf_result, double cdf_result);
void app_state_clear_parameters(app_state_t* state);
const double* app_state_get_parameters(const app_state_t* state);
uint8_t app_state_get_parameter_count(const app_state_t* state);

// Additional state management functions
int app_state_validate(const app_state_t* state);
void app_state_reset(app_state_t* state);
distribution_category_t app_state_get_category(const app_state_t* state);
distribution_type_t app_state_get_distribution(const app_state_t* state);
int app_state_parameters_complete(const app_state_t* state);

#endif // APP_STATE_H