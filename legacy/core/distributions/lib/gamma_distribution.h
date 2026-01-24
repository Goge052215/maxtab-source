#ifndef GAMMA_DISTRIBUTION_H
#define GAMMA_DISTRIBUTION_H

#include "distribution_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Gamma distribution PDF calculation
 * @param x The value at which to evaluate the PDF
 * @param params Array containing [shape, scale]
 * @param param_count Number of parameters (should be 2)
 * @return PDF value at x
 */
double gamma_pdf(double x, double* params, int param_count);

/**
 * @brief Gamma distribution CDF calculation
 * @param x The value at which to evaluate the CDF
 * @param params Array containing [shape, scale]
 * @param param_count Number of parameters (should be 2)
 * @return CDF value at x
 */
double gamma_cdf(double x, double* params, int param_count);

/**
 * @brief Validate Gamma distribution parameters
 * @param params Array containing [shape, scale]
 * @param param_count Number of parameters (should be 2)
 * @return 1 if parameters are valid, 0 otherwise
 */
int gamma_validate_params(double* params, int param_count);

/**
 * @brief Get the Gamma distribution interface
 * @return Pointer to the distribution_t structure for Gamma distribution
 */
const distribution_t* get_gamma_distribution(void);

#ifdef __cplusplus
}
#endif

#endif // GAMMA_DISTRIBUTION_H