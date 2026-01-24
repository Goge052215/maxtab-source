#ifndef BETA_DISTRIBUTION_H
#define BETA_DISTRIBUTION_H

#include "distribution_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Beta distribution PDF calculation
 * @param x The value at which to evaluate the PDF
 * @param params Array containing [alpha, beta]
 * @param param_count Number of parameters (should be 2)
 * @return PDF value at x
 */
double beta_pdf(double x, double* params, int param_count);

/**
 * @brief Beta distribution CDF calculation
 * @param x The value at which to evaluate the CDF
 * @param params Array containing [alpha, beta]
 * @param param_count Number of parameters (should be 2)
 * @return CDF value at x
 */
double beta_cdf(double x, double* params, int param_count);

/**
 * @brief Validate Beta distribution parameters
 * @param params Array containing [alpha, beta]
 * @param param_count Number of parameters (should be 2)
 * @return 1 if parameters are valid, 0 otherwise
 */
int beta_validate_params(double* params, int param_count);

/**
 * @brief Get the Beta distribution interface
 * @return Pointer to the distribution_t structure for Beta distribution
 */
const distribution_t* get_beta_distribution(void);

#ifdef __cplusplus
}
#endif

#endif // BETA_DISTRIBUTION_H