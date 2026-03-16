declare module 'jstat' {
    interface JStat {
        // Core Statistics
        mean(data: number[]): number;
        stdev(data: number[], sample?: boolean): number;
        variance(data: number[], sample?: boolean): number;
        sum(data: number[]): number;
        pooledvariance(arrays: number[][]): number;
        pooledstdev(arrays: number[][]): number;

        // Confidence Intervals
        normalci(value: number, alpha: number, sd: number, n: number): [number, number];
        normalci(value: number, alpha: number, array: number[]): [number, number];
        tci(value: number, alpha: number, sd: number, n: number): [number, number];
        tci(value: number, alpha: number, array: number[]): [number, number];

        // Hypothesis Tests
        ztest(value: number, array: number[], sides: number, flag?: boolean): number; // p-value
        ztest(value: number, mean: number, sd: number, sides: number): number;
        ttest(value: number, array: number[], sides: number): number;
        ttest(value: number, mean: number, sd: number, n: number, sides: number): number;
        ftest(fscore: number, df1: number, df2: number): number; // p-value
        anovaftest(...arrays: number[][]): number; // p-value for ANOVA (usable for two-sample equal var t)

        // Distributions (trimmed to used ones)
        normal: {
            pdf(x: number, mean: number, std: number): number;
            cdf(x: number, mean: number, std: number): number;
            inv(p: number, mean: number, std: number): number;
            sample(mean: number, std: number): number;
        };
        studentt: {
            pdf(x: number, df: number): number;
            cdf(x: number, df: number): number;
            inv(p: number, df: number): number;
        };
        centralF: {
            pdf(x: number, df1: number, df2: number): number;
            cdf(x: number, df1: number, df2: number): number;
            inv(p: number, df1: number, df2: number): number;
        };
        chisquare: {
            pdf(x: number, df: number): number;
            cdf(x: number, df: number): number;
            inv(p: number, df: number): number;
        };
        beta: {
            pdf(x: number, alpha: number, beta: number): number;
            cdf(x: number, alpha: number, beta: number): number;
        };
        gamma: {
            pdf(x: number, shape: number, scale: number): number;
            cdf(x: number, shape: number, scale: number): number;
        };
        negbin: {
            pdf(k: number, r: number, p: number): number;
            cdf(k: number, r: number, p: number): number;
        };
        hypgeom: {
            pdf(k: number, N: number, K: number, n: number): number;
            cdf(k: number, N: number, K: number, n: number): number;
        };

        // Helper
        factorial(n: number): number;
        combination(n: number, k: number): number;

        // Linear Algebra (for regression)
        corrcoeff(array1: number[], array2: number[]): number;
        lstsq(A: number[][], b: number[]): number[]; // Least squares solution
    }

    const jstat: JStat;
    export default jstat;
}
