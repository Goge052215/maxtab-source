#ifndef DISTRIBUTION_REGISTRY_H
#define DISTRIBUTION_REGISTRY_H

#include "../../core/distributions/lib/distribution_interface.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Distribution registry entry structure
 */
typedef struct {
    distribution_type_t type;
    const char* name;
    const char* description;
    distribution_category_t category;
    uint8_t param_count;
    const char** param_names;
    double param_ranges[MAX_PARAMETERS][2];  // [min, max] for each parameter
    const distribution_t* distribution_impl;
} distribution_registry_entry_t;

/**
 * @brief Distribution registry structure
 */
typedef struct {
    const distribution_registry_entry_t* entries;
    uint8_t total_count;
    uint8_t continuous_count;
    uint8_t discrete_count;
} distribution_registry_t;

/**
 * @brief Registry initialization and access functions
 */
const distribution_registry_t* get_distribution_registry(void);
const distribution_registry_entry_t* registry_get_distribution(distribution_type_t type);
const distribution_registry_entry_t* registry_get_distribution_by_index(uint8_t index);
const distribution_registry_entry_t** registry_get_distributions_by_category(distribution_category_t category, uint8_t* count);
uint8_t registry_get_total_count(void);
uint8_t registry_get_category_count(distribution_category_t category);
int registry_is_valid_distribution_type(distribution_type_t type);

/**
 * @brief Distribution metadata access functions
 */
const char* registry_get_distribution_name(distribution_type_t type);
const char* registry_get_distribution_description(distribution_type_t type);
distribution_category_t registry_get_distribution_category(distribution_type_t type);
uint8_t registry_get_parameter_count(distribution_type_t type);
const char** registry_get_parameter_names(distribution_type_t type);
const double* registry_get_parameter_ranges(distribution_type_t type, uint8_t param_index);

#ifdef __cplusplus
}
#endif

#endif // DISTRIBUTION_REGISTRY_H