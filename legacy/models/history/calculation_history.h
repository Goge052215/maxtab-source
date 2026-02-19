#ifndef CALCULATION_HISTORY_H
#define CALCULATION_HISTORY_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Maximum number of history entries to store
 */
#define MAX_HISTORY_ENTRIES 10

/**
 * @brief Maximum number of parameters for any distribution
 */
#define MAX_PARAMETERS 4

/**
 * @brief Calculation history entry structure
 */
typedef struct {
    uint32_t timestamp;
    uint8_t distribution_type;
    double parameters[MAX_PARAMETERS];
    double input_value;
    double pdf_result;
    double cdf_result;
} calculation_entry_t;

/**
 * @brief Calculation history management structure
 */
typedef struct {
    calculation_entry_t entries[MAX_HISTORY_ENTRIES];
    uint8_t count;
    uint8_t head;  // Circular buffer head index
} calculation_history_t;

/**
 * @brief Function prototypes for history management
 */
void history_init(calculation_history_t* history);
int history_add_entry(calculation_history_t* history, const calculation_entry_t* entry);
const calculation_entry_t* history_get_entry(const calculation_history_t* history, uint8_t index);
void history_clear(calculation_history_t* history);
uint8_t history_get_count(const calculation_history_t* history);

/**
 * @brief Function prototypes for entry creation and timestamp management
 */
int history_create_entry(uint8_t distribution_type, const double* parameters, 
                        uint8_t param_count, double input_value, 
                        double pdf_result, double cdf_result, 
                        calculation_entry_t* entry);

/**
 * @brief Function prototypes for serialization
 */
int history_serialize(const calculation_history_t* history, uint8_t* buffer, size_t buffer_size, size_t* bytes_written);
int history_deserialize(calculation_history_t* history, const uint8_t* buffer, size_t buffer_size);
size_t history_get_serialized_size(const calculation_history_t* history);

#endif // CALCULATION_HISTORY_H