#include "calculation_orchestrator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// QuickApp native plugin interface
// This file provides the bridge between JavaScript and C functions

/**
 * @brief Native plugin entry point for statistical calculations
 * @param method The method name to call (should be "orchestrator_calculate_with_request")
 * @param params JSON string containing the calculation request
 * @return JSON string containing the calculation result
 */
const char* orchestrator_calculate_with_request_plugin(const char* params_json) {
    if (!params_json) {
        return "{\"success\": 0, \"error_message\": \"Invalid parameters\"}";
    }
    
    // For now, return a simple test response to verify the connection
    // In a full implementation, you would parse the JSON params and call the C functions
    static char result_json[512];
    snprintf(result_json, sizeof(result_json), 
             "{\"success\": 1, \"pdf_result\": 0.246, \"cdf_result\": 0.623, \"error_message\": null}");
    
    return result_json;
}

/**
 * @brief Plugin initialization function
 * Called when the native plugin is loaded
 */
void initialize_statistical_calculator_plugin(void) {
    // Initialize any global state or resources needed by the plugin
    printf("Statistical Calculator Plugin initialized\n");
}

/**
 * @brief Plugin cleanup function
 * Called when the native plugin is unloaded
 */
void cleanup_statistical_calculator_plugin(void) {
    // Clean up any resources
    printf("Statistical Calculator Plugin cleaned up\n");
}

// Plugin export table for QuickApp
// This structure tells QuickApp which functions are available
typedef struct {
    const char* name;
    void* function;
} plugin_function_t;

static plugin_function_t plugin_functions[] = {
    {"orchestrator_calculate_with_request", (void*)orchestrator_calculate_with_request_plugin},
    {"initialize", (void*)initialize_statistical_calculator_plugin},
    {"cleanup", (void*)cleanup_statistical_calculator_plugin},
    {NULL, NULL} // Terminator
};

/**
 * @brief Get plugin function table
 * @return Pointer to the function table
 */
plugin_function_t* get_plugin_functions(void) {
    return plugin_functions;
}
