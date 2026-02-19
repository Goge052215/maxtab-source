#ifndef HISTORY_PERSISTENCE_H
#define HISTORY_PERSISTENCE_H

#include "calculation_history.h"

/**
 * @brief Default filename for history storage
 */
#define HISTORY_FILENAME "calc_history.dat"

/**
 * @brief Maximum path length for history file
 */
#define MAX_PATH_LENGTH 256

/**
 * @brief Function prototypes for history persistence
 */
int history_save_to_file(const calculation_history_t* history, const char* filename);
int history_load_from_file(calculation_history_t* history, const char* filename);
int history_file_exists(const char* filename);
int history_delete_file(const char* filename);

/**
 * @brief Convenience functions using default filename
 */
int history_save(const calculation_history_t* history);
int history_load(calculation_history_t* history);
int history_file_exists_default(void);
int history_delete_file_default(void);

#endif // HISTORY_PERSISTENCE_H