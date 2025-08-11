#include "calculation_history.h"
#include <string.h>
#include <time.h>
#include <stddef.h>

/**
 * @brief Initialize calculation history structure
 * @param history Pointer to history structure to initialize
 */
void history_init(calculation_history_t* history) {
    if (history == NULL) {
        return;
    }
    
    memset(history, 0, sizeof(calculation_history_t));
    history->count = 0;
    history->head = 0;
}

/**
 * @brief Add a new calculation entry to history (circular buffer)
 * @param history Pointer to history structure
 * @param entry Pointer to calculation entry to add
 * @return 0 on success, -1 on error
 */
int history_add_entry(calculation_history_t* history, const calculation_entry_t* entry) {
    if (history == NULL || entry == NULL) {
        return -1;
    }
    
    // Copy entry to current head position
    memcpy(&history->entries[history->head], entry, sizeof(calculation_entry_t));
    
    // Update head position (circular buffer)
    history->head = (history->head + 1) % MAX_HISTORY_ENTRIES;
    
    // Update count (max is MAX_HISTORY_ENTRIES)
    if (history->count < MAX_HISTORY_ENTRIES) {
        history->count++;
    }
    
    return 0;
}

/**
 * @brief Get calculation entry by index (0 = most recent)
 * @param history Pointer to history structure
 * @param index Index of entry to retrieve (0 = most recent)
 * @return Pointer to entry or NULL if invalid index
 */
const calculation_entry_t* history_get_entry(const calculation_history_t* history, uint8_t index) {
    if (history == NULL || index >= history->count) {
        return NULL;
    }
    
    // Calculate actual array index for circular buffer
    // Most recent entry is at (head - 1), second most recent at (head - 2), etc.
    int actual_index = (history->head - 1 - index + MAX_HISTORY_ENTRIES) % MAX_HISTORY_ENTRIES;
    
    return &history->entries[actual_index];
}

/**
 * @brief Clear all history entries
 * @param history Pointer to history structure
 */
void history_clear(calculation_history_t* history) {
    if (history == NULL) {
        return;
    }
    
    memset(history->entries, 0, sizeof(history->entries));
    history->count = 0;
    history->head = 0;
}

/**
 * @brief Get current number of history entries
 * @param history Pointer to history structure
 * @return Number of entries in history
 */
uint8_t history_get_count(const calculation_history_t* history) {
    if (history == NULL) {
        return 0;
    }
    
    return history->count;
}

/**
 * @brief Create a new calculation entry with current timestamp
 * @param distribution_type Type of distribution used
 * @param parameters Array of distribution parameters
 * @param param_count Number of parameters
 * @param input_value Input value for calculation
 * @param pdf_result PDF calculation result
 * @param cdf_result CDF calculation result
 * @param entry Pointer to entry structure to fill
 * @return 0 on success, -1 on error
 */
int history_create_entry(uint8_t distribution_type, const double* parameters, 
                        uint8_t param_count, double input_value, 
                        double pdf_result, double cdf_result, 
                        calculation_entry_t* entry) {
    if (entry == NULL || parameters == NULL || param_count > MAX_PARAMETERS) {
        return -1;
    }
    
    // Set timestamp (using time() for now, could be platform-specific)
    entry->timestamp = (uint32_t)time(NULL);
    entry->distribution_type = distribution_type;
    entry->input_value = input_value;
    entry->pdf_result = pdf_result;
    entry->cdf_result = cdf_result;
    
    // Copy parameters
    memset(entry->parameters, 0, sizeof(entry->parameters));
    for (uint8_t i = 0; i < param_count && i < MAX_PARAMETERS; i++) {
        entry->parameters[i] = parameters[i];
    }
    
    return 0;
}

/**
 * @brief Get the size needed for serialized history data
 * @param history Pointer to history structure
 * @return Size in bytes needed for serialization
 */
size_t history_get_serialized_size(const calculation_history_t* history) {
    if (history == NULL) {
        return 0;
    }
    
    // Size = header (count + head) + entries
    return sizeof(uint8_t) + sizeof(uint8_t) + 
           (history->count * sizeof(calculation_entry_t));
}

/**
 * @brief Serialize history data to binary buffer
 * @param history Pointer to history structure
 * @param buffer Buffer to write serialized data
 * @param buffer_size Size of the buffer
 * @param bytes_written Pointer to store number of bytes written
 * @return 0 on success, -1 on error
 */
int history_serialize(const calculation_history_t* history, uint8_t* buffer, 
                     size_t buffer_size, size_t* bytes_written) {
    if (history == NULL || buffer == NULL || bytes_written == NULL) {
        return -1;
    }
    
    size_t required_size = history_get_serialized_size(history);
    if (buffer_size < required_size) {
        return -1;
    }
    
    uint8_t* ptr = buffer;
    
    // Write header information
    *ptr++ = history->count;
    *ptr++ = history->head;
    
    // Write entries in chronological order (oldest to newest)
    for (uint8_t i = 0; i < history->count; i++) {
        const calculation_entry_t* entry = history_get_entry(history, history->count - 1 - i);
        if (entry != NULL) {
            memcpy(ptr, entry, sizeof(calculation_entry_t));
            ptr += sizeof(calculation_entry_t);
        }
    }
    
    *bytes_written = ptr - buffer;
    return 0;
}

/**
 * @brief Deserialize history data from binary buffer
 * @param history Pointer to history structure to populate
 * @param buffer Buffer containing serialized data
 * @param buffer_size Size of the buffer
 * @return 0 on success, -1 on error
 */
int history_deserialize(calculation_history_t* history, const uint8_t* buffer, 
                       size_t buffer_size) {
    if (history == NULL || buffer == NULL || buffer_size < 2) {
        return -1;
    }
    
    const uint8_t* ptr = buffer;
    
    // Read header information
    uint8_t count = *ptr++;
    uint8_t head = *ptr++;
    
    // Validate header data
    if (count > MAX_HISTORY_ENTRIES || head >= MAX_HISTORY_ENTRIES) {
        return -1;
    }
    
    // Check if buffer has enough data for all entries
    size_t expected_size = 2 + (count * sizeof(calculation_entry_t));
    if (buffer_size < expected_size) {
        return -1;
    }
    
    // Initialize history structure
    history_init(history);
    
    // Read entries and add them in chronological order
    for (uint8_t i = 0; i < count; i++) {
        calculation_entry_t entry;
        memcpy(&entry, ptr, sizeof(calculation_entry_t));
        ptr += sizeof(calculation_entry_t);
        
        // Add entry to history
        if (history_add_entry(history, &entry) != 0) {
            return -1;
        }
    }
    
    return 0;
}