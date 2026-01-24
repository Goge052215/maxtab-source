#ifndef NEGATIVE_BINOMIAL_DISTRIBUTION_H
#define NEGATIVE_BINOMIAL_DISTRIBUTION_H

#include "distribution_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Negative Binomial distribution PDF calculation
 * @param x The value at which to evaluate the PDF (must be non-negative integer)
 * @param params Array containing [r, p] where:
 *               r = number of successes (positive integer)
 *               p = probability of success on each trial (0 < p <= 1)
 * @param param_count Number of parameters (should be 2)
 * @return PDF value at x
 */
double negative_binomial_pdf(double x, double* params, int param_count);

/**
 * @brief Negative Binomial distribution CDF calculation
 * @param x The value at which to evaluate the CDF
 * @param params Array containing [r, p] where:
 *               r = number of successes (positive integer)
 *               p = probability of success on each trial (0 < p <= 1)
 * @param param_count Number of parameters (should be 2)
 * @return CDF value at x
 */
double negative_binomial_cdf(double x, double* params, int param_count);

/**
 * @brief Validate Negative Binomial distribution parameters
 * @param params Array containing [r, p] where:
 *               r > 0 (number of successes, positive integer)
 *               0 < p <= 1 (probability of success)
 * @param param_count Number of parameters (should be 2)
 * @return 1 if parameters are valid, 0 otherwise
 */
int negative_binomial_validate_params(double* params, int param_count);

/**
 * @brief Get the Negative Binomial distribution interface
 * @return Pointer to the distribution_t structure for Negative Binomial distribution
 */
const distribution_t* get_negative_binomial_distribution(void);

#ifdef __cplusplus
}
#endif

#endif // NEGATIVE_BINOMIAL_DISTRIBUTION_H