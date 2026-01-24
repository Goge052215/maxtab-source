#include <stdio.h>
#include <math.h>

// Error function implementation for generating table
double erf_approx(double x) {
    static const double a1 =  0.254829592;
    static const double a2 = -0.284496736;
    static const double a3 =  1.421413741;
    static const double a4 = -1.453152027;
    static const double a5 =  1.061405429;
    static const double p  =  0.3275911;
    
    if (x == 0.0) return 0.0;
    
    int sign = (x >= 0) ? 1 : -1;
    x = fabs(x);
    
    double t = 1.0 / (1.0 + p * x);
    double y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * exp(-x * x);
    
    return sign * y;
}

double normal_cdf(double z) {
    return 0.5 * (1.0 + erf_approx(z / sqrt(2.0)));
}

int main() {
    printf("// Normal CDF values for z from -4.0 to 4.0, step 0.02\n");
    printf("const double normal_cdf_values[401] = {\n");
    
    for (int i = 0; i < 401; i++) {
        double z = -4.0 + i * 0.02;
        double cdf_val = normal_cdf(z);
        printf("    %.8f", cdf_val);
        if (i < 400) printf(",");
        if (i % 5 == 4) printf("\n");
    }
    
    printf("};\n");
    return 0;
}