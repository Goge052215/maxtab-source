#ifndef WEIBULL_DISTRIBUTION_H
#define WEIBULL_DISTRIBUTION_H

#include "distribution_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Weibull distribution PDF calculation
 * @param x The value at which to evaluate the PDF
 * @param params Array containing [shape, scale]
 * @param param_count Number of parameters (should be 2)
 * @return PDF value at x
 */
double weibull_pdf(double x, double* params, int param_count);

/**
 * @brief Weibull distribution CDF calculation
 * @param x The value at which to evaluate the CDF
 * @param params Array containing [shape, scale]
 * @param param_count Number of parameters (should be 2)
 * @return CDF value at x
 */
double weibull_cdf(double x, double* params, int param_count);

/**
 * @brief Validate Weibull distribution parameters
 * @param params Array containing [shape, scale]
 * @param param_count Number of parameters (should be 2)
 * @return 1 if parameters are valid, 0 otherwise
 */
int weibull_validate_params(double* params, int param_count);

/**
 * @brief Get the Weibull distribution interface
 * @return Pointer to the distribution_t structure for Weibull distribution
 */
const distribution_t* get_weibull_distribution(void);

#ifdef __cplusplus
}
#endif

#endif // WEIBULL_DISTRIBUTION_H