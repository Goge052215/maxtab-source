#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include "calculation_history.h"
#include "history_persistence.h"

/**
 * @brief History manager structure that combines history and persistence
 */
typedef struct {
    calculation_history_t history;
    char storage_path[MAX_PATH_LENGTH];
    int auto_save;  // 1 = auto-save on add, 0 = manual save
} history_manager_t;

/**
 * @brief Function prototypes for history management
 */
int history_manager_init(history_manager_t* manager, const char* storage_path, int auto_save);
int history_manager_add_calculation(history_manager_t* manager, uint8_t distribution_type,
                                   const double* parameters, uint8_t param_count,
                                   double input_value, double pdf_result, double cdf_result);
const calculation_entry_t* history_manager_get_entry(const history_manager_t* manager, uint8_t index);
uint8_t history_manager_get_count(const history_manager_t* manager);
int history_manager_clear(history_manager_t* manager);
int history_manager_save(const history_manager_t* manager);
int history_manager_load(history_manager_t* manager);
int history_manager_remove_entry(history_manager_t* manager, uint8_t index);

#endif // HISTORY_MANAGER_H