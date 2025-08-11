#ifndef HYPERGEOMETRIC_DISTRIBUTION_H
#define HYPERGEOMETRIC_DISTRIBUTION_H

#include "distribution_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Hypergeometric distribution PDF calculation
 * @param x The value at which to evaluate the PDF (must be non-negative integer)
 * @param params Array containing [N, K, n] where:
 *               N = population size
 *               K = number of success states in population
 *               n = number of draws (sample size)
 * @param param_count Number of parameters (should be 3)
 * @return PDF value at x
 */
double hypergeometric_pdf(double x, double* params, int param_count);

/**
 * @brief Hypergeometric distribution CDF calculation
 * @param x The value at which to evaluate the CDF
 * @param params Array containing [N, K, n] where:
 *               N = population size
 *               K = number of success states in population
 *               n = number of draws (sample size)
 * @param param_count Number of parameters (should be 3)
 * @return CDF value at x
 */
double hypergeometric_cdf(double x, double* params, int param_count);

/**
 * @brief Validate Hypergeometric distribution parameters
 * @param params Array containing [N, K, n] where:
 *               N >= 1 (population size)
 *               0 <= K <= N (success states)
 *               0 <= n <= N (sample size)
 * @param param_count Number of parameters (should be 3)
 * @return 1 if parameters are valid, 0 otherwise
 */
int hypergeometric_validate_params(double* params, int param_count);

/**
 * @brief Get the Hypergeometric distribution interface
 * @return Pointer to the distribution_t structure for Hypergeometric distribution
 */
const distribution_t* get_hypergeometric_distribution(void);

#ifdef __cplusplus
}
#endif

#endif // HYPERGEOMETRIC_DISTRIBUTION_H