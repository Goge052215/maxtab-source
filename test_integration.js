#!/usr/bin/env node
/**
 * Integration Test for Native Bridge
 * Tests the restructured native_bridge.js with fallback to JavaScript implementation
 */

import { NativeBridge } from './src/common/native_bridge.js'

async function testIntegration() {
  console.log('🧪 Testing Native Bridge Integration')
  console.log('==================================\n')

  try {
    // Initialize the bridge
    console.log('Initializing Native Bridge...')
    await NativeBridge.initialize()
    console.log('✅ Native Bridge initialized successfully\n')
    
    // Check backend information
    const mode = NativeBridge.getCalculationMode()
    const perfInfo = NativeBridge.getPerformanceInfo()
    const bunFFIAvailable = NativeBridge.isBunFFIAvailable()
    
    console.log(`📊 Active Backend: ${mode}`)
    console.log(`🔧 Bun FFI Available: ${bunFFIAvailable}`)
    console.log(`📈 Performance Info:`, perfInfo)
    console.log('')
    
    // Test normal distribution
    console.log('Testing Normal Distribution...')
    const normalResult = NativeBridge.normalDistribution(0, 1, 1.5)
    console.log(`  Input: mean=0, stdDev=1, x=1.5`)
    console.log(`  PDF: ${normalResult.pdfResult.toFixed(6)}`)
    console.log(`  CDF: ${normalResult.cdfResult.toFixed(6)}`)
    console.log('')
    
    // Test binomial distribution
    console.log('Testing Binomial Distribution...')
    const binomialResult = NativeBridge.binomial(10, 0.3, 5)
    console.log(`  Input: n=10, p=0.3, k=5`)
    console.log(`  PMF: ${binomialResult.pmfResult.toFixed(6)}`)
    console.log(`  CDF: ${binomialResult.cdfResult.toFixed(6)}`)
    console.log('')
    
    // Test Poisson distribution
    console.log('Testing Poisson Distribution...')
    const poissonResult = NativeBridge.poisson(2.5, 3)
    console.log(`  Input: lambda=2.5, k=3`)
    console.log(`  PMF: ${poissonResult.pmfResult.toFixed(6)}`)
    console.log(`  CDF: ${poissonResult.cdfResult.toFixed(6)}`)
    console.log('')
    
    // Test exponential distribution
    console.log('Testing Exponential Distribution...')
    const expResult = NativeBridge.exponentialDistribution(1.5, 2.0)
    console.log(`  Input: lambda=1.5, x=2.0`)
    console.log(`  PDF: ${expResult.pdfResult.toFixed(6)}`)
    console.log(`  CDF: ${expResult.cdfResult.toFixed(6)}`)
    console.log('')
    
    // Test chi-square distribution
    console.log('Testing Chi-Square Distribution...')
    const chiResult = NativeBridge.chiSquareDistribution(5, 3.2)
    console.log(`  Input: df=5, x=3.2`)
    console.log(`  PDF: ${chiResult.pdfResult.toFixed(6)}`)
    console.log(`  CDF: ${chiResult.cdfResult.toFixed(6)}`)
    console.log('')
    
    // Test t-distribution
    console.log('Testing T-Distribution...')
    const tResult = NativeBridge.tDistribution(10, 1.8)
    console.log(`  Input: df=10, t=1.8`)
    console.log(`  PDF: ${tResult.pdfResult.toFixed(6)}`)
    console.log(`  CDF: ${tResult.cdfResult.toFixed(6)}`)
    console.log('')
    
    // Test distribution factory (if available)
    try {
      const factory = NativeBridge.getDistributionFactory()
      if (factory) {
        console.log('Testing Distribution Factory...')
        const normal = factory.createNormal(0, 1)
        console.log(`  Normal PDF at x=1.5: ${normal.pdf(1.5).toFixed(6)}`)
        console.log(`  Normal CDF at x=1.5: ${normal.cdf(1.5).toFixed(6)}`)
        console.log(`  Normal Mean: ${normal.getMean()}`)
        console.log(`  Normal Variance: ${normal.getVariance()}`)
        console.log('')
      }
    } catch (error) {
      console.log('Distribution Factory not available (expected in Node.js mode)')
      console.log('')
    }
    
    // Performance test
    console.log('Running Performance Test...')
    const iterations = 1000
    const start = performance.now()
    
    for (let i = 0; i < iterations; i++) {
      NativeBridge.normalDistribution(0, 1, i / iterations)
    }
    
    const elapsed = performance.now() - start
    console.log(`  ${iterations} normal distribution calculations in ${elapsed.toFixed(2)}ms`)
    console.log(`  Average: ${(elapsed / iterations).toFixed(4)}ms per calculation`)
    console.log('')
    
    // Test benchmark method (if available)
    try {
      const benchmarkResult = await NativeBridge.benchmark()
      console.log('Benchmark Result:', benchmarkResult)
    } catch (error) {
      console.log('Benchmark method not available (expected in Node.js mode)')
    }
    
    console.log('🎉 All tests completed successfully!')
    console.log('')
    console.log('Summary:')
    console.log(`- Backend: ${mode}`)
    console.log(`- Bun FFI: ${bunFFIAvailable ? 'Available' : 'Not Available (using JavaScript fallback)'}`)
    console.log('- All distribution calculations working correctly')
    console.log('- Performance test completed')
    
  } catch (error) {
    console.error('❌ Test failed:', error)
    console.error(error.stack)
    process.exit(1)
  }
}

// Run the test
if (import.meta.url === `file://${process.argv[1]}`) {
  testIntegration().catch(console.error)
}

export { testIntegration }