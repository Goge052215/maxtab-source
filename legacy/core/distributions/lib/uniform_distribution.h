#ifndef UNIFORM_DISTRIBUTION_H
#define UNIFORM_DISTRIBUTION_H

#include "distribution_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Uniform distribution PDF calculation
 * @param x The value at which to evaluate the PDF
 * @param params Array containing [a, b]
 * @param param_count Number of parameters (should be 2)
 * @return PDF value at x
 */
double uniform_pdf(double x, double* params, int param_count);

/**
 * @brief Uniform distribution CDF calculation
 * @param x The value at which to evaluate the CDF
 * @param params Array containing [a, b]
 * @param param_count Number of parameters (should be 2)
 * @return CDF value at x
 */
double uniform_cdf(double x, double* params, int param_count);

/**
 * @brief Validate Uniform distribution parameters
 * @param params Array containing [a, b]
 * @param param_count Number of parameters (should be 2)
 * @return 1 if parameters are valid, 0 otherwise
 */
int uniform_validate_params(double* params, int param_count);

/**
 * @brief Get the Uniform distribution interface
 * @return Pointer to the distribution_t structure for Uniform distribution
 */
const distribution_t* get_uniform_distribution(void);

#ifdef __cplusplus
}
#endif

#endif // UNIFORM_DISTRIBUTION_H