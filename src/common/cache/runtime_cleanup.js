const runtimeCleanupCallbacks = new Set()

/** @param {() => void} callback */
export function registerRuntimeCleanup(callback) {
  if (typeof callback !== 'function') {
    return () => {}
  }

  runtimeCleanupCallbacks.add(callback)

  return () => {
    runtimeCleanupCallbacks.delete(callback)
  }
}

export function runRuntimeCleanup() {
  runtimeCleanupCallbacks.forEach((callback) => {
    try {
      callback()
    } catch (error) {
      // Ignore cleanup failures so remaining callbacks still run.
    }
  })
}
