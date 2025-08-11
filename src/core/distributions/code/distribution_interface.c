#include "../lib/distribution_interface.h"
#include "../../../models/distributions/distribution_registry.h"
#include <stddef.h>

/**
 * @brief Get distribution implementation by type
 * @param type Distribution type
 * @return Pointer to distribution implementation, or NULL if not found
 */
const distribution_t* get_distribution(distribution_type_t type) {
    const distribution_registry_entry_t* entry = registry_get_distribution(type);
    return entry ? entry->distribution_impl : NULL;
}

/**
 * @brief Get distribution model information by type
 * @param type Distribution type
 * @return Pointer to distribution model, or NULL if not found
 */
const distribution_model_t* get_distribution_model(distribution_type_t type) {
    const distribution_registry_entry_t* entry = registry_get_distribution(type);
    if (!entry) {
        return NULL;
    }
    
    // Create a static model structure from registry entry
    static distribution_model_t model;
    model.distribution_id = (uint8_t)type;
    model.name = entry->name;
    model.param_count = entry->param_count;
    model.param_names = entry->param_names;
    model.category = entry->category;
    
    // Copy parameter ranges
    for (int i = 0; i < 4; i++) {
        model.param_ranges[i][0] = entry->param_ranges[i][0];
        model.param_ranges[i][1] = entry->param_ranges[i][1];
    }
    
    return &model;
}

/**
 * @brief Check if distribution type is valid
 * @param type Distribution type to validate
 * @return 1 if valid, 0 if invalid
 */
int is_valid_distribution_type(distribution_type_t type) {
    return registry_is_valid_distribution_type(type);
}