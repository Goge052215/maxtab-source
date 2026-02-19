#include "app_state.h"
#include "../../core/distributions/lib/distribution_interface.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief Initialize application state with default values
 * @param state Pointer to application state structure
 */
void app_state_init(app_state_t* state) {
    if (!state) {
        return;
    }
    
    // Initialize with first continuous distribution (Normal)
    state->current_distribution = DIST_NORMAL;
    state->current_category = DISTRIBUTION_CONTINUOUS;
    
    // Set parameter count based on distribution
    const distribution_model_t* model = get_distribution_model(DIST_NORMAL);
    state->parameter_count = model ? model->param_count : 0;
    
    // Clear all parameters
    memset(state->current_parameters, 0, sizeof(state->current_parameters));
    
    // Initialize calculation history
    history_init(&state->history);
    
    // Mark as initialized
    state->is_initialized = 1;
}

/**
 * @brief Set the current distribution and update related state
 * @param state Pointer to application state structure
 * @param distribution Distribution type to set
 * @return 0 on success, -1 on error
 */
int app_state_set_distribution(app_state_t* state, distribution_type_t distribution) {
    if (!state || !state->is_initialized) {
        return -1;
    }
    
    // Validate distribution type
    if (!is_valid_distribution_type(distribution)) {
        return -1;
    }
    
    // Get distribution model to update category and parameter count
    const distribution_model_t* model = get_distribution_model(distribution);
    if (!model) {
        return -1;
    }
    
    // Update state
    state->current_distribution = distribution;
    state->current_category = model->category;
    state->parameter_count = model->param_count;
    
    // Clear previous parameters when switching distributions
    app_state_clear_parameters(state);
    
    return 0;
}

/**
 * @brief Set a parameter value at the specified index
 * @param state Pointer to application state structure
 * @param param_index Index of the parameter (0-based)
 * @param value Parameter value to set
 * @return 0 on success, -1 on error
 */
int app_state_set_parameter(app_state_t* state, uint8_t param_index, double value) {
    if (!state || !state->is_initialized) {
        return -1;
    }
    
    // Validate parameter index
    if (param_index >= MAX_PARAMETERS) {
        return -1;
    }
    
    // Get distribution model for validation
    const distribution_model_t* model = get_distribution_model(state->current_distribution);
    if (!model || param_index >= model->param_count) {
        return -1;
    }
    
    // Validate parameter range
    double min_val = model->param_ranges[param_index][0];
    double max_val = model->param_ranges[param_index][1];
    
    if (value < min_val || value > max_val) {
        return -1;
    }
    
    // Set parameter value
    state->current_parameters[param_index] = value;
    
    return 0;
}

/**
 * @brief Add a calculation result to the history
 * @param state Pointer to application state structure
 * @param input_value Input value used for calculation
 * @param pdf_result PDF calculation result
 * @param cdf_result CDF calculation result
 * @return 0 on success, -1 on error
 */
int app_state_add_calculation(app_state_t* state, double input_value, double pdf_result, double cdf_result) {
    if (!state || !state->is_initialized) {
        return -1;
    }
    
    // Create calculation entry
    calculation_entry_t entry;
    entry.distribution_type = state->current_distribution;
    entry.input_value = input_value;
    entry.pdf_result = pdf_result;
    entry.cdf_result = cdf_result;
    
    // Copy current parameters
    memcpy(entry.parameters, state->current_parameters, sizeof(entry.parameters));
    
    // Add to history
    return history_add_entry(&state->history, &entry);
}

/**
 * @brief Clear all current parameters
 * @param state Pointer to application state structure
 */
void app_state_clear_parameters(app_state_t* state) {
    if (!state || !state->is_initialized) {
        return;
    }
    
    memset(state->current_parameters, 0, sizeof(state->current_parameters));
}

/**
 * @brief Get pointer to current parameters array
 * @param state Pointer to application state structure
 * @return Pointer to parameters array, or NULL on error
 */
const double* app_state_get_parameters(const app_state_t* state) {
    if (!state || !state->is_initialized) {
        return NULL;
    }
    
    return state->current_parameters;
}

/**
 * @brief Get current parameter count for the selected distribution
 * @param state Pointer to application state structure
 * @return Parameter count, or 0 on error
 */
uint8_t app_state_get_parameter_count(const app_state_t* state) {
    if (!state || !state->is_initialized) {
        return 0;
    }
    
    return state->parameter_count;
}

/**
 * @brief Validate current application state
 * @param state Pointer to application state structure
 * @return 0 if valid, -1 if invalid
 */
int app_state_validate(const app_state_t* state) {
    if (!state || !state->is_initialized) {
        return -1;
    }
    
    // Validate distribution type
    if (!is_valid_distribution_type(state->current_distribution)) {
        return -1;
    }
    
    // Validate category consistency
    const distribution_model_t* model = get_distribution_model(state->current_distribution);
    if (!model || model->category != state->current_category) {
        return -1;
    }
    
    // Validate parameter count
    if (state->parameter_count != model->param_count) {
        return -1;
    }
    
    return 0;
}

/**
 * @brief Reset application state to default values
 * @param state Pointer to application state structure
 */
void app_state_reset(app_state_t* state) {
    if (!state) {
        return;
    }
    
    // Clear history
    history_clear(&state->history);
    
    // Reinitialize state
    app_state_init(state);
}

/**
 * @brief Get current distribution category
 * @param state Pointer to application state structure
 * @return Current distribution category
 */
distribution_category_t app_state_get_category(const app_state_t* state) {
    if (!state || !state->is_initialized) {
        return DISTRIBUTION_CONTINUOUS; // Default fallback
    }
    
    return state->current_category;
}

/**
 * @brief Get current distribution type
 * @param state Pointer to application state structure
 * @return Current distribution type
 */
distribution_type_t app_state_get_distribution(const app_state_t* state) {
    if (!state || !state->is_initialized) {
        return DIST_NORMAL; // Default fallback
    }
    
    return state->current_distribution;
}

/**
 * @brief Check if all required parameters are set for current distribution
 * @param state Pointer to application state structure
 * @return 1 if all parameters are set, 0 otherwise
 */
int app_state_parameters_complete(const app_state_t* state) {
    if (!state || !state->is_initialized) {
        return 0;
    }
    
    const distribution_model_t* model = get_distribution_model(state->current_distribution);
    if (!model) {
        return 0;
    }
    
    // Check if we have the required number of parameters
    // For simplicity, we assume parameters are set sequentially
    for (uint8_t i = 0; i < model->param_count; i++) {
        // Check if parameter is within valid range (non-zero check is basic validation)
        double min_val = model->param_ranges[i][0];
        double max_val = model->param_ranges[i][1];
        
        if (state->current_parameters[i] < min_val || state->current_parameters[i] > max_val) {
            return 0;
        }
    }
    
    return 1;
}