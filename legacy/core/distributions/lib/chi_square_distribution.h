#ifndef CHI_SQUARE_DISTRIBUTION_H
#define CHI_SQUARE_DISTRIBUTION_H

#include "distribution_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Chi-Square distribution PDF calculation
 * @param x The value at which to evaluate the PDF
 * @param params Array containing [degrees_of_freedom]
 * @param param_count Number of parameters (should be 1)
 * @return PDF value at x
 */
double chi_square_pdf(double x, double* params, int param_count);

/**
 * @brief Chi-Square distribution CDF calculation
 * @param x The value at which to evaluate the CDF
 * @param params Array containing [degrees_of_freedom]
 * @param param_count Number of parameters (should be 1)
 * @return CDF value at x
 */
double chi_square_cdf(double x, double* params, int param_count);

/**
 * @brief Validate Chi-Square distribution parameters
 * @param params Array containing [degrees_of_freedom]
 * @param param_count Number of parameters (should be 1)
 * @return 1 if parameters are valid, 0 otherwise
 */
int chi_square_validate_params(double* params, int param_count);

/**
 * @brief Get the Chi-Square distribution interface
 * @return Pointer to the distribution_t structure for Chi-Square distribution
 */
const distribution_t* get_chi_square_distribution(void);

#ifdef __cplusplus
}
#endif

#endif // CHI_SQUARE_DISTRIBUTION_H