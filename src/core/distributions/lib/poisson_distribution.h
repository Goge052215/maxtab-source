#ifndef POISSON_DISTRIBUTION_H
#define POISSON_DISTRIBUTION_H

#include "distribution_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Poisson distribution PDF calculation
 * @param x The value at which to evaluate the PDF (must be non-negative integer)
 * @param params Array containing [lambda] where:
 *               lambda = rate parameter (average number of events)
 * @param param_count Number of parameters (should be 1)
 * @return PDF value at x
 */
double poisson_pdf(double x, double* params, int param_count);

/**
 * @brief Poisson distribution CDF calculation
 * @param x The value at which to evaluate the CDF
 * @param params Array containing [lambda] where:
 *               lambda = rate parameter (average number of events)
 * @param param_count Number of parameters (should be 1)
 * @return CDF value at x
 */
double poisson_cdf(double x, double* params, int param_count);

/**
 * @brief Validate Poisson distribution parameters
 * @param params Array containing [lambda] where:
 *               lambda > 0 (rate parameter)
 * @param param_count Number of parameters (should be 1)
 * @return 1 if parameters are valid, 0 otherwise
 */
int poisson_validate_params(double* params, int param_count);

/**
 * @brief Get the Poisson distribution interface
 * @return Pointer to the distribution_t structure for Poisson distribution
 */
const distribution_t* get_poisson_distribution(void);

#ifdef __cplusplus
}
#endif

#endif // POISSON_DISTRIBUTION_H