#!/usr/bin/env bun
/**
 * Benchmark Test Script
 * Demonstrates the performance comparison between C FFI and JavaScript implementations
 */

import { runComprehensiveBenchmark, quickBenchmark, benchmarkMemoryUsage } from '../core/performance_benchmarks.js'
import { NativeBridge } from '../common/native_bridge.js'

// ============================================================================
// TEST CONFIGURATION
// ============================================================================

const TEST_MODES = {
  quick: 'Quick benchmark (1,000 iterations)',
  standard: 'Standard benchmark (10,000 iterations)', 
  intensive: 'Intensive benchmark (100,000 iterations)',
  memory: 'Memory usage benchmark'
}

// ============================================================================
// MAIN TEST FUNCTION
// ============================================================================

async function runBenchmarkTest() {
  console.log('🚀 Statistical Distribution Benchmark Test')
  console.log('==========================================\n')

  // Initialize the native bridge
  console.log('Initializing Native Bridge...')
  try {
    await NativeBridge.initialize()
    console.log('✅ Native Bridge initialized successfully')
    
    // Display backend information
    const mode = NativeBridge.getCalculationMode()
    const perfInfo = NativeBridge.getPerformanceInfo()
    
    console.log(`📊 Active Backend: ${mode}`)
    console.log(`🔧 Performance Info:`, perfInfo)
    console.log('')
    
  } catch (error) {
    console.error('❌ Failed to initialize Native Bridge:', error)
    process.exit(1)
  }

  // Check if Bun FFI is available
  if (!NativeBridge.isBunFFIAvailable()) {
    console.warn('⚠️  Bun FFI not available - running JavaScript-only benchmarks')
  } else {
    console.log('✅ Bun FFI available - running comparative benchmarks')
  }

  console.log('')

  // Get test mode from command line arguments
  const args = process.argv.slice(2)
  const testMode = args[0] || 'quick'

  if (!TEST_MODES[testMode]) {
    console.error(`❌ Invalid test mode: ${testMode}`)
    console.log('Available modes:', Object.keys(TEST_MODES).join(', '))
    process.exit(1)
  }

  console.log(`🎯 Running: ${TEST_MODES[testMode]}\n`)

  try {
    let report

    switch (testMode) {
      case 'quick':
        report = await quickBenchmark()
        break
      case 'standard':
        report = await runComprehensiveBenchmark(10000)
        break
      case 'intensive':
        report = await runComprehensiveBenchmark(100000)
        break
      case 'memory':
        const memoryResults = await benchmarkMemoryUsage()
        console.log('Memory Usage Results:')
        console.log(JSON.stringify(memoryResults, null, 2))
        return
      default:
        report = await quickBenchmark()
    }

    // Display results
    console.log(report.toString())

    // Save results to file
    const timestamp = new Date().toISOString().replace(/[:.]/g, '-')
    const filename = `/Users/goge/maxtab/benchmark_results_${timestamp}.json`
    
    try {
      await Bun.write(filename, JSON.stringify(report.toJSON(), null, 2))
      console.log(`📁 Results saved to: ${filename}`)
    } catch (error) {
      console.warn('⚠️  Could not save results to file:', error.message)
    }

    // Performance summary
    const avgSpeedup = report.getAverageSpeedup()
    const avgAccuracy = report.getAverageAccuracy()

    console.log('\n🎉 Benchmark Summary:')
    if (avgSpeedup > 1) {
      console.log(`   C FFI is ${avgSpeedup.toFixed(2)}x faster on average`)
    } else if (avgSpeedup < 1 && avgSpeedup > 0) {
      console.log(`   JavaScript is ${(1/avgSpeedup).toFixed(2)}x faster on average`)
    } else {
      console.log('   Performance comparison not available')
    }
    
    console.log(`   Average accuracy: ${(avgAccuracy * 100).toFixed(2)}%`)
    
    if (avgAccuracy < 0.99) {
      console.warn('⚠️  Accuracy below 99% - check implementation differences')
    }

  } catch (error) {
    console.error('❌ Benchmark failed:', error)
    console.error(error.stack)
    process.exit(1)
  }
}

// ============================================================================
// INDIVIDUAL DISTRIBUTION TESTS
// ============================================================================

async function testIndividualDistribution() {
  console.log('🧪 Testing Individual Distribution Performance\n')
  
  await NativeBridge.initialize()
  
  // Test normal distribution with different parameters
  const testCases = [
    { name: 'Standard Normal', params: [0, 1, 0] },
    { name: 'Wide Normal', params: [0, 5, 2] },
    { name: 'Shifted Normal', params: [10, 2, 12] }
  ]
  
  for (const testCase of testCases) {
    console.log(`Testing ${testCase.name}...`)
    
    // JavaScript implementation
    const jsStart = performance.now()
    const jsResult = NativeBridge.normalDistribution(...testCase.params)
    const jsTime = performance.now() - jsStart
    
    // FFI implementation (if available)
    let ffiTime = 0
    let ffiResult = null
    
    if (NativeBridge.isBunFFIAvailable()) {
      const engine = NativeBridge.getBunFFIEngine()
      const ffiStart = performance.now()
      ffiResult = engine.calculateNormal(...testCase.params)
      ffiTime = performance.now() - ffiStart
    }
    
    console.log(`  JavaScript: ${jsTime.toFixed(4)}ms, Result: PDF=${jsResult.pdfResult.toFixed(6)}, CDF=${jsResult.cdfResult.toFixed(6)}`)
    
    if (ffiResult) {
      console.log(`  Bun FFI:    ${ffiTime.toFixed(4)}ms, Result: PDF=${ffiResult.pdfResult.toFixed(6)}, CDF=${ffiResult.cdfResult.toFixed(6)}`)
      console.log(`  Speedup:    ${(jsTime / ffiTime).toFixed(2)}x`)
    }
    
    console.log('')
  }
}

// ============================================================================
// USAGE EXAMPLES
// ============================================================================

function printUsage() {
  console.log('Usage: bun benchmark_test.js [mode]')
  console.log('')
  console.log('Available modes:')
  Object.entries(TEST_MODES).forEach(([key, description]) => {
    console.log(`  ${key.padEnd(10)} - ${description}`)
  })
  console.log('')
  console.log('Examples:')
  console.log('  bun benchmark_test.js quick      # Quick test with 1K iterations')
  console.log('  bun benchmark_test.js standard   # Standard test with 10K iterations')
  console.log('  bun benchmark_test.js intensive  # Intensive test with 100K iterations')
  console.log('  bun benchmark_test.js memory     # Memory usage comparison')
}

// ============================================================================
// MAIN EXECUTION
// ============================================================================

if (import.meta.main) {
  const args = process.argv.slice(2)
  
  if (args.includes('--help') || args.includes('-h')) {
    printUsage()
    process.exit(0)
  }
  
  if (args.includes('--individual')) {
    testIndividualDistribution().catch(console.error)
  } else {
    runBenchmarkTest().catch(console.error)
  }
}

export { runBenchmarkTest, testIndividualDistribution }