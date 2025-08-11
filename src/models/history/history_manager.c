#include "history_manager.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief Initialize history manager
 * @param manager Pointer to history manager structure
 * @param storage_path Path for persistent storage (NULL for default)
 * @param auto_save 1 for auto-save on add, 0 for manual save
 * @return 0 on success, -1 on error
 */
int history_manager_init(history_manager_t* manager, const char* storage_path, int auto_save) {
    if (manager == NULL) {
        return -1;
    }
    
    // Initialize history structure
    history_init(&manager->history);
    
    // Set storage path
    if (storage_path != NULL) {
        strncpy(manager->storage_path, storage_path, MAX_PATH_LENGTH - 1);
        manager->storage_path[MAX_PATH_LENGTH - 1] = '\0';
    } else {
        strcpy(manager->storage_path, HISTORY_FILENAME);
    }
    
    manager->auto_save = auto_save;
    
    return 0;
}

/**
 * @brief Add a new calculation to history
 * @param manager Pointer to history manager
 * @param distribution_type Type of distribution used
 * @param parameters Array of distribution parameters
 * @param param_count Number of parameters
 * @param input_value Input value for calculation
 * @param pdf_result PDF calculation result
 * @param cdf_result CDF calculation result
 * @return 0 on success, -1 on error
 */
int history_manager_add_calculation(history_manager_t* manager, uint8_t distribution_type,
                                   const double* parameters, uint8_t param_count,
                                   double input_value, double pdf_result, double cdf_result) {
    if (manager == NULL) {
        return -1;
    }
    
    // Create calculation entry
    calculation_entry_t entry;
    int result = history_create_entry(distribution_type, parameters, param_count,
                                    input_value, pdf_result, cdf_result, &entry);
    if (result != 0) {
        return -1;
    }
    
    // Add to history
    result = history_add_entry(&manager->history, &entry);
    if (result != 0) {
        return -1;
    }
    
    // Auto-save if enabled
    if (manager->auto_save) {
        history_save_to_file(&manager->history, manager->storage_path);
        // Note: We don't return error if save fails, as the calculation was added successfully
    }
    
    return 0;
}

/**
 * @brief Get calculation entry by index
 * @param manager Pointer to history manager
 * @param index Index of entry to retrieve (0 = most recent)
 * @return Pointer to entry or NULL if invalid index
 */
const calculation_entry_t* history_manager_get_entry(const history_manager_t* manager, uint8_t index) {
    if (manager == NULL) {
        return NULL;
    }
    
    return history_get_entry(&manager->history, index);
}

/**
 * @brief Get current number of history entries
 * @param manager Pointer to history manager
 * @return Number of entries in history
 */
uint8_t history_manager_get_count(const history_manager_t* manager) {
    if (manager == NULL) {
        return 0;
    }
    
    return history_get_count(&manager->history);
}

/**
 * @brief Clear all history entries
 * @param manager Pointer to history manager
 * @return 0 on success, -1 on error
 */
int history_manager_clear(history_manager_t* manager) {
    if (manager == NULL) {
        return -1;
    }
    
    history_clear(&manager->history);
    
    // Auto-save if enabled
    if (manager->auto_save) {
        history_save_to_file(&manager->history, manager->storage_path);
    }
    
    return 0;
}

/**
 * @brief Save history to persistent storage
 * @param manager Pointer to history manager
 * @return 0 on success, -1 on error
 */
int history_manager_save(const history_manager_t* manager) {
    if (manager == NULL) {
        return -1;
    }
    
    return history_save_to_file(&manager->history, manager->storage_path);
}

/**
 * @brief Load history from persistent storage
 * @param manager Pointer to history manager
 * @return 0 on success, -1 on error
 */
int history_manager_load(history_manager_t* manager) {
    if (manager == NULL) {
        return -1;
    }
    
    return history_load_from_file(&manager->history, manager->storage_path);
}

/**
 * @brief Remove specific entry from history (not implemented for circular buffer)
 * @param manager Pointer to history manager
 * @param index Index of entry to remove
 * @return -1 (not supported for circular buffer implementation)
 */
int history_manager_remove_entry(history_manager_t* manager, uint8_t index) {
    // Note: Removing specific entries from a circular buffer is complex
    // and not typically needed for this use case. The circular buffer
    // automatically removes oldest entries when full.
    (void)manager;  // Suppress unused parameter warning
    (void)index;    // Suppress unused parameter warning
    return -1;  // Not implemented
}