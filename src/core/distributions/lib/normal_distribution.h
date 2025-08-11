#ifndef NORMAL_DISTRIBUTION_H
#define NORMAL_DISTRIBUTION_H

#include "distribution_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Normal distribution PDF calculation
 * @param x The value at which to evaluate the PDF
 * @param params Array containing [mean, standard_deviation]
 * @param param_count Number of parameters (should be 2)
 * @return PDF value at x
 */
double normal_pdf(double x, double* params, int param_count);

/**
 * @brief Normal distribution CDF calculation
 * @param x The value at which to evaluate the CDF
 * @param params Array containing [mean, standard_deviation]
 * @param param_count Number of parameters (should be 2)
 * @return CDF value at x
 */
double normal_cdf(double x, double* params, int param_count);

/**
 * @brief Validate Normal distribution parameters
 * @param params Array containing [mean, standard_deviation]
 * @param param_count Number of parameters (should be 2)
 * @return 1 if parameters are valid, 0 otherwise
 */
int normal_validate_params(double* params, int param_count);

/**
 * @brief Get the Normal distribution interface
 * @return Pointer to the distribution_t structure for Normal distribution
 */
const distribution_t* get_normal_distribution(void);

#ifdef __cplusplus
}
#endif

#endif // NORMAL_DISTRIBUTION_H