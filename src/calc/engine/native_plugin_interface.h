#ifndef NATIVE_PLUGIN_INTERFACE_H
#define NATIVE_PLUGIN_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Native plugin entry point for statistical calculations
 * @param params_json JSON string containing the calculation request
 * @return JSON string containing the calculation result
 */
const char* orchestrator_calculate_with_request_plugin(const char* params_json);

/**
 * @brief Plugin initialization function
 */
void initialize_statistical_calculator_plugin(void);

/**
 * @brief Plugin cleanup function
 */
void cleanup_statistical_calculator_plugin(void);

/**
 * @brief Plugin function structure
 */
typedef struct {
    const char* name;
    void* function;
} plugin_function_t;

/**
 * @brief Get plugin function table
 * @return Pointer to the function table
 */
plugin_function_t* get_plugin_functions(void);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_PLUGIN_INTERFACE_H
