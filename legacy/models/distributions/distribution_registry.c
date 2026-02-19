#include "distribution_registry.h"
#include "../../core/distributions/lib/normal_distribution.h"
#include "../../core/distributions/lib/exponential_distribution.h"
#include "../../core/distributions/lib/chi_square_distribution.h"
#include "../../core/distributions/lib/t_distribution.h"
#include "../../core/distributions/lib/f_distribution.h"
#include "../../core/distributions/lib/geometric_distribution.h"
#include "../../core/distributions/lib/hypergeometric_distribution.h"
#include "../../core/distributions/lib/binomial_distribution.h"
#include "../../core/distributions/lib/negative_binomial_distribution.h"
#include "../../core/distributions/lib/poisson_distribution.h"
#include <stddef.h>

// Parameter names for each distribution
static const char* normal_param_names[] = {"mean", "std_dev"};
static const char* exponential_param_names[] = {"lambda"};
static const char* chi_square_param_names[] = {"degrees_of_freedom"};
static const char* t_param_names[] = {"degrees_of_freedom"};
static const char* f_param_names[] = {"df_numerator", "df_denominator"};
static const char* geometric_param_names[] = {"probability"};
static const char* hypergeometric_param_names[] = {"population_size", "success_states", "sample_size"};
static const char* binomial_param_names[] = {"trials", "probability"};
static const char* negative_binomial_param_names[] = {"successes", "probability"};
static const char* poisson_param_names[] = {"lambda"};

// Distribution registry entries
static distribution_registry_entry_t registry_entries[] = {
    // Continuous distributions
    {
        .type = DIST_NORMAL,
        .name = "Normal",
        .description = "Normal (Gaussian) distribution",
        .category = DISTRIBUTION_CONTINUOUS,
        .param_count = 2,
        .param_names = normal_param_names,
        .param_ranges = {{-1000.0, 1000.0}, {0.001, 1000.0}, {0.0, 0.0}, {0.0, 0.0}},
        .distribution_impl = NULL  // Will be set during initialization
    },
    {
        .type = DIST_EXPONENTIAL,
        .name = "Exponential",
        .description = "Exponential distribution",
        .category = DISTRIBUTION_CONTINUOUS,
        .param_count = 1,
        .param_names = exponential_param_names,
        .param_ranges = {{0.001, 1000.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}},
        .distribution_impl = NULL
    },
    {
        .type = DIST_CHI_SQUARE,
        .name = "Chi-Square",
        .description = "Chi-square distribution",
        .category = DISTRIBUTION_CONTINUOUS,
        .param_count = 1,
        .param_names = chi_square_param_names,
        .param_ranges = {{1.0, 1000.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}},
        .distribution_impl = NULL
    },
    {
        .type = DIST_T_DISTRIBUTION,
        .name = "t-Distribution",
        .description = "Student's t-distribution",
        .category = DISTRIBUTION_CONTINUOUS,
        .param_count = 1,
        .param_names = t_param_names,
        .param_ranges = {{1.0, 1000.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}},
        .distribution_impl = NULL
    },
    {
        .type = DIST_F_DISTRIBUTION,
        .name = "F-Distribution",
        .description = "F-distribution",
        .category = DISTRIBUTION_CONTINUOUS,
        .param_count = 2,
        .param_names = f_param_names,
        .param_ranges = {{1.0, 1000.0}, {1.0, 1000.0}, {0.0, 0.0}, {0.0, 0.0}},
        .distribution_impl = NULL
    },
    
    // Discrete distributions
    {
        .type = DIST_GEOMETRIC,
        .name = "Geometric",
        .description = "Geometric distribution",
        .category = DISTRIBUTION_DISCRETE,
        .param_count = 1,
        .param_names = geometric_param_names,
        .param_ranges = {{0.001, 0.999}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}},
        .distribution_impl = NULL
    },
    {
        .type = DIST_HYPERGEOMETRIC,
        .name = "Hypergeometric",
        .description = "Hypergeometric distribution",
        .category = DISTRIBUTION_DISCRETE,
        .param_count = 3,
        .param_names = hypergeometric_param_names,
        .param_ranges = {{1.0, 10000.0}, {0.0, 10000.0}, {1.0, 10000.0}, {0.0, 0.0}},
        .distribution_impl = NULL
    },
    {
        .type = DIST_BINOMIAL,
        .name = "Binomial",
        .description = "Binomial distribution",
        .category = DISTRIBUTION_DISCRETE,
        .param_count = 2,
        .param_names = binomial_param_names,
        .param_ranges = {{1.0, 10000.0}, {0.001, 0.999}, {0.0, 0.0}, {0.0, 0.0}},
        .distribution_impl = NULL
    },
    {
        .type = DIST_NEGATIVE_BINOMIAL,
        .name = "Negative Binomial",
        .description = "Negative binomial distribution",
        .category = DISTRIBUTION_DISCRETE,
        .param_count = 2,
        .param_names = negative_binomial_param_names,
        .param_ranges = {{1.0, 10000.0}, {0.001, 0.999}, {0.0, 0.0}, {0.0, 0.0}},
        .distribution_impl = NULL
    },
    {
        .type = DIST_POISSON,
        .name = "Poisson",
        .description = "Poisson distribution",
        .category = DISTRIBUTION_DISCRETE,
        .param_count = 1,
        .param_names = poisson_param_names,
        .param_ranges = {{0.001, 1000.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}},
        .distribution_impl = NULL
    }
};

// Registry structure
static distribution_registry_t registry = {
    .entries = registry_entries,
    .total_count = DIST_COUNT,
    .continuous_count = 5,  // Normal, Exponential, Chi-Square, t-Distribution, F-Distribution
    .discrete_count = 5     // Geometric, Hypergeometric, Binomial, Negative Binomial, Poisson
};

// Static arrays for category-based lookups
static const distribution_registry_entry_t* continuous_distributions[5];
static const distribution_registry_entry_t* discrete_distributions[5];
static int registry_initialized = 0;

/**
 * @brief Initialize the registry with distribution implementations
 */
static void initialize_registry(void) {
    if (registry_initialized) {
        return;
    }
    
    // Initialize distribution implementations
    // Connect C distribution implementations to registry entries
    for (uint8_t i = 0; i < registry.total_count; i++) {
        switch (registry_entries[i].type) {
            case DIST_NORMAL:
                registry_entries[i].distribution_impl = get_normal_distribution();
                break;
            case DIST_EXPONENTIAL:
                registry_entries[i].distribution_impl = get_exponential_distribution();
                break;
            case DIST_CHI_SQUARE:
                registry_entries[i].distribution_impl = get_chi_square_distribution();
                break;
            case DIST_T_DISTRIBUTION:
                registry_entries[i].distribution_impl = get_t_distribution();
                break;
            case DIST_F_DISTRIBUTION:
                registry_entries[i].distribution_impl = get_f_distribution();
                break;
            case DIST_GEOMETRIC:
                registry_entries[i].distribution_impl = get_geometric_distribution();
                break;
            case DIST_HYPERGEOMETRIC:
                registry_entries[i].distribution_impl = get_hypergeometric_distribution();
                break;
            case DIST_BINOMIAL:
                registry_entries[i].distribution_impl = get_binomial_distribution();
                break;
            case DIST_NEGATIVE_BINOMIAL:
                registry_entries[i].distribution_impl = get_negative_binomial_distribution();
                break;
            case DIST_POISSON:
                registry_entries[i].distribution_impl = get_poisson_distribution();
                break;
            default:
                registry_entries[i].distribution_impl = NULL;
                break;
        }
    }
    
    // Populate category arrays
    uint8_t continuous_idx = 0;
    uint8_t discrete_idx = 0;
    
    for (uint8_t i = 0; i < registry.total_count; i++) {
        if (registry_entries[i].category == DISTRIBUTION_CONTINUOUS) {
            continuous_distributions[continuous_idx++] = &registry_entries[i];
        } else {
            discrete_distributions[discrete_idx++] = &registry_entries[i];
        }
    }
    
    registry_initialized = 1;
}

const distribution_registry_t* get_distribution_registry(void) {
    initialize_registry();
    return &registry;
}

const distribution_registry_entry_t* registry_get_distribution(distribution_type_t type) {
    initialize_registry();
    
    if (type >= DIST_COUNT) {
        return NULL;
    }
    
    // Find the entry with matching type
    for (uint8_t i = 0; i < registry.total_count; i++) {
        if (registry_entries[i].type == type) {
            return &registry_entries[i];
        }
    }
    
    return NULL;
}

const distribution_registry_entry_t* registry_get_distribution_by_index(uint8_t index) {
    initialize_registry();
    
    if (index >= registry.total_count) {
        return NULL;
    }
    
    return &registry_entries[index];
}

const distribution_registry_entry_t** registry_get_distributions_by_category(distribution_category_t category, uint8_t* count) {
    initialize_registry();
    
    if (category == DISTRIBUTION_CONTINUOUS) {
        if (count) *count = registry.continuous_count;
        return continuous_distributions;
    } else if (category == DISTRIBUTION_DISCRETE) {
        if (count) *count = registry.discrete_count;
        return discrete_distributions;
    }
    
    if (count) *count = 0;
    return NULL;
}

uint8_t registry_get_total_count(void) {
    return registry.total_count;
}

uint8_t registry_get_category_count(distribution_category_t category) {
    if (category == DISTRIBUTION_CONTINUOUS) {
        return registry.continuous_count;
    } else if (category == DISTRIBUTION_DISCRETE) {
        return registry.discrete_count;
    }
    return 0;
}

int registry_is_valid_distribution_type(distribution_type_t type) {
    return (type < DIST_COUNT);
}

const char* registry_get_distribution_name(distribution_type_t type) {
    const distribution_registry_entry_t* entry = registry_get_distribution(type);
    return entry ? entry->name : NULL;
}

const char* registry_get_distribution_description(distribution_type_t type) {
    const distribution_registry_entry_t* entry = registry_get_distribution(type);
    return entry ? entry->description : NULL;
}

distribution_category_t registry_get_distribution_category(distribution_type_t type) {
    const distribution_registry_entry_t* entry = registry_get_distribution(type);
    return entry ? entry->category : DISTRIBUTION_CONTINUOUS;  // Default fallback
}

uint8_t registry_get_parameter_count(distribution_type_t type) {
    const distribution_registry_entry_t* entry = registry_get_distribution(type);
    return entry ? entry->param_count : 0;
}

const char** registry_get_parameter_names(distribution_type_t type) {
    const distribution_registry_entry_t* entry = registry_get_distribution(type);
    return entry ? entry->param_names : NULL;
}

const double* registry_get_parameter_ranges(distribution_type_t type, uint8_t param_index) {
    const distribution_registry_entry_t* entry = registry_get_distribution(type);
    if (!entry || param_index >= entry->param_count) {
        return NULL;
    }
    return entry->param_ranges[param_index];
}