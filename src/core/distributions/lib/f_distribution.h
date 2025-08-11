#ifndef F_DISTRIBUTION_H
#define F_DISTRIBUTION_H

#include "distribution_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief F-distribution PDF calculation
 * @param x The value at which to evaluate the PDF
 * @param params Array containing [numerator_df, denominator_df]
 * @param param_count Number of parameters (should be 2)
 * @return PDF value at x
 */
double f_pdf(double x, double* params, int param_count);

/**
 * @brief F-distribution CDF calculation
 * @param x The value at which to evaluate the CDF
 * @param params Array containing [numerator_df, denominator_df]
 * @param param_count Number of parameters (should be 2)
 * @return CDF value at x
 */
double f_cdf(double x, double* params, int param_count);

/**
 * @brief Validate F-distribution parameters
 * @param params Array containing [numerator_df, denominator_df]
 * @param param_count Number of parameters (should be 2)
 * @return 1 if parameters are valid, 0 otherwise
 */
int f_validate_params(double* params, int param_count);

/**
 * @brief Get the F-distribution interface
 * @return Pointer to the distribution_t structure for F-distribution
 */
const distribution_t* get_f_distribution(void);

#ifdef __cplusplus
}
#endif

#endif // F_DISTRIBUTION_H