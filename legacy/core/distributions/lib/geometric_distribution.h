#ifndef GEOMETRIC_DISTRIBUTION_H
#define GEOMETRIC_DISTRIBUTION_H

#include "distribution_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Geometric distribution PDF calculation
 * @param x The value at which to evaluate the PDF (must be positive integer)
 * @param params Array containing [p] where p is probability of success
 * @param param_count Number of parameters (should be 1)
 * @return PDF value at x
 */
double geometric_pdf(double x, double* params, int param_count);

/**
 * @brief Geometric distribution CDF calculation
 * @param x The value at which to evaluate the CDF
 * @param params Array containing [p] where p is probability of success
 * @param param_count Number of parameters (should be 1)
 * @return CDF value at x
 */
double geometric_cdf(double x, double* params, int param_count);

/**
 * @brief Validate Geometric distribution parameters
 * @param params Array containing [p] where 0 < p <= 1
 * @param param_count Number of parameters (should be 1)
 * @return 1 if parameters are valid, 0 otherwise
 */
int geometric_validate_params(double* params, int param_count);

/**
 * @brief Get the Geometric distribution interface
 * @return Pointer to the distribution_t structure for Geometric distribution
 */
const distribution_t* get_geometric_distribution(void);

#ifdef __cplusplus
}
#endif

#endif // GEOMETRIC_DISTRIBUTION_H