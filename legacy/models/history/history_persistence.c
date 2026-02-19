#include "history_persistence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Save calculation history to file
 * @param history Pointer to history structure
 * @param filename Path to file for saving
 * @return 0 on success, -1 on error
 */
int history_save_to_file(const calculation_history_t* history, const char* filename) {
    if (history == NULL || filename == NULL) {
        return -1;
    }
    
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        return -1;
    }
    
    // Get serialized size
    size_t serialized_size = history_get_serialized_size(history);
    if (serialized_size == 0) {
        fclose(file);
        return -1;
    }
    
    // Allocate buffer for serialization
    uint8_t* buffer = malloc(serialized_size);
    if (buffer == NULL) {
        fclose(file);
        return -1;
    }
    
    // Serialize history
    size_t bytes_written;
    int result = history_serialize(history, buffer, serialized_size, &bytes_written);
    if (result != 0) {
        free(buffer);
        fclose(file);
        return -1;
    }
    
    // Write to file
    size_t written = fwrite(buffer, 1, bytes_written, file);
    
    free(buffer);
    fclose(file);
    
    return (written == bytes_written) ? 0 : -1;
}

/**
 * @brief Load calculation history from file
 * @param history Pointer to history structure to populate
 * @param filename Path to file for loading
 * @return 0 on success, -1 on error
 */
int history_load_from_file(calculation_history_t* history, const char* filename) {
    if (history == NULL || filename == NULL) {
        return -1;
    }
    
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        return -1;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0 || file_size > 65536) {  // Reasonable size limit
        fclose(file);
        return -1;
    }
    
    // Allocate buffer for file data
    uint8_t* buffer = malloc(file_size);
    if (buffer == NULL) {
        fclose(file);
        return -1;
    }
    
    // Read file data
    size_t read_bytes = fread(buffer, 1, file_size, file);
    fclose(file);
    
    if (read_bytes != (size_t)file_size) {
        free(buffer);
        return -1;
    }
    
    // Deserialize history
    int result = history_deserialize(history, buffer, read_bytes);
    
    free(buffer);
    return result;
}

/**
 * @brief Check if history file exists
 * @param filename Path to file to check
 * @return 1 if file exists, 0 if not, -1 on error
 */
int history_file_exists(const char* filename) {
    if (filename == NULL) {
        return -1;
    }
    
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        return 0;
    }
    
    fclose(file);
    return 1;
}

/**
 * @brief Delete history file
 * @param filename Path to file to delete
 * @return 0 on success, -1 on error
 */
int history_delete_file(const char* filename) {
    if (filename == NULL) {
        return -1;
    }
    
    return remove(filename);
}

/**
 * @brief Save history using default filename
 * @param history Pointer to history structure
 * @return 0 on success, -1 on error
 */
int history_save(const calculation_history_t* history) {
    return history_save_to_file(history, HISTORY_FILENAME);
}

/**
 * @brief Load history using default filename
 * @param history Pointer to history structure to populate
 * @return 0 on success, -1 on error
 */
int history_load(calculation_history_t* history) {
    return history_load_from_file(history, HISTORY_FILENAME);
}

/**
 * @brief Check if default history file exists
 * @return 1 if file exists, 0 if not, -1 on error
 */
int history_file_exists_default(void) {
    return history_file_exists(HISTORY_FILENAME);
}

/**
 * @brief Delete default history file
 * @return 0 on success, -1 on error
 */
int history_delete_file_default(void) {
    return history_delete_file(HISTORY_FILENAME);
}