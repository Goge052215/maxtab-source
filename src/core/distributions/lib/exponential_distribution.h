#ifndef EXPONENTIAL_DISTRIBUTION_H
#define EXPONENTIAL_DISTRIBUTION_H

#include "distribution_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Exponential distribution PDF calculation
 * @param x The value at which to evaluate the PDF
 * @param params Array containing [lambda] (rate parameter)
 * @param param_count Number of parameters (should be 1)
 * @return PDF value at x
 */
double exponential_pdf(double x, double* params, int param_count);

/**
 * @brief Exponential distribution CDF calculation
 * @param x The value at which to evaluate the CDF
 * @param params Array containing [lambda] (rate parameter)
 * @param param_count Number of parameters (should be 1)
 * @return CDF value at x
 */
double exponential_cdf(double x, double* params, int param_count);

/**
 * @brief Validate Exponential distribution parameters
 * @param params Array containing [lambda] (rate parameter)
 * @param param_count Number of parameters (should be 1)
 * @return 1 if parameters are valid, 0 otherwise
 */
int exponential_validate_params(double* params, int param_count);

/**
 * @brief Get the Exponential distribution interface
 * @return Pointer to the distribution_t structure for Exponential distribution
 */
const distribution_t* get_exponential_distribution(void);

#ifdef __cplusplus
}
#endif

#endif // EXPONENTIAL_DISTRIBUTION_H