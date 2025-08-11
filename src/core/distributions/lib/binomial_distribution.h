#ifndef BINOMIAL_DISTRIBUTION_H
#define BINOMIAL_DISTRIBUTION_H

#include "distribution_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Binomial distribution PDF calculation
 * @param x The value at which to evaluate the PDF (must be non-negative integer)
 * @param params Array containing [n, p] where:
 *               n = number of trials
 *               p = probability of success on each trial
 * @param param_count Number of parameters (should be 2)
 * @return PDF value at x
 */
double binomial_pdf(double x, double* params, int param_count);

/**
 * @brief Binomial distribution CDF calculation
 * @param x The value at which to evaluate the CDF
 * @param params Array containing [n, p] where:
 *               n = number of trials
 *               p = probability of success on each trial
 * @param param_count Number of parameters (should be 2)
 * @return CDF value at x
 */
double binomial_cdf(double x, double* params, int param_count);

/**
 * @brief Validate Binomial distribution parameters
 * @param params Array containing [n, p] where:
 *               n >= 0 (number of trials, integer)
 *               0 <= p <= 1 (probability of success)
 * @param param_count Number of parameters (should be 2)
 * @return 1 if parameters are valid, 0 otherwise
 */
int binomial_validate_params(double* params, int param_count);

/**
 * @brief Get the Binomial distribution interface
 * @return Pointer to the distribution_t structure for Binomial distribution
 */
const distribution_t* get_binomial_distribution(void);

#ifdef __cplusplus
}
#endif

#endif // BINOMIAL_DISTRIBUTION_H