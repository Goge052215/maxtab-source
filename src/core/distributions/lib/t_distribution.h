#ifndef T_DISTRIBUTION_H
#define T_DISTRIBUTION_H

#include "distribution_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Student's t-distribution PDF calculation
 * @param x The value at which to evaluate the PDF
 * @param params Array containing [degrees_of_freedom]
 * @param param_count Number of parameters (should be 1)
 * @return PDF value at x
 */
double t_pdf(double x, double* params, int param_count);

/**
 * @brief Student's t-distribution CDF calculation
 * @param x The value at which to evaluate the CDF
 * @param params Array containing [degrees_of_freedom]
 * @param param_count Number of parameters (should be 1)
 * @return CDF value at x
 */
double t_cdf(double x, double* params, int param_count);

/**
 * @brief Validate Student's t-distribution parameters
 * @param params Array containing [degrees_of_freedom]
 * @param param_count Number of parameters (should be 1)
 * @return 1 if parameters are valid, 0 otherwise
 */
int t_validate_params(double* params, int param_count);

/**
 * @brief Get the Student's t-distribution interface
 * @return Pointer to the distribution_t structure for t-distribution
 */
const distribution_t* get_t_distribution(void);

#ifdef __cplusplus
}
#endif

#endif // T_DISTRIBUTION_H