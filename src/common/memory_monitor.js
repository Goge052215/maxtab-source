/**
 * Memory Monitor for Vela OS - MaxTab Application
 * Provides memory usage monitoring and optimization capabilities
 */

class MemoryMonitor {
    constructor() {
        this.isEnabled = true;
        this.monitoringInterval = null;
        this.memoryStats = {
            heapUsed: 0,
            heapTotal: 0,
            external: 0,
            timestamp: Date.now()
        };
        this.thresholds = {
            warning: 50 * 1024 * 1024, // 50MB
            critical: 100 * 1024 * 1024, // 100MB
            cleanup: 80 * 1024 * 1024 // 80MB
        };
        this.callbacks = {
            warning: [],
            critical: [],
            cleanup: []
        };
        
        this.init();
    }
    
    init() {
        // Start monitoring if memory API is available
        if (this.isMemoryAPIAvailable()) {
            this.startMonitoring();
        }
        
        // Setup cleanup triggers
        this.setupCleanupTriggers();
    }
    
    isMemoryAPIAvailable() {
        return (
            typeof performance !== 'undefined' && 
            performance.memory
        ) || (
            typeof process !== 'undefined' && 
            process.memoryUsage
        );
    }
    
    getCurrentMemoryUsage() {
        try {
            if (typeof performance !== 'undefined' && performance.memory) {
                // Browser environment
                return {
                    heapUsed: performance.memory.usedJSHeapSize,
                    heapTotal: performance.memory.totalJSHeapSize,
                    external: 0,
                    timestamp: Date.now()
                };
            } else if (typeof process !== 'undefined' && process.memoryUsage) {
                // Node.js environment
                const usage = process.memoryUsage();
                return {
                    heapUsed: usage.heapUsed,
                    heapTotal: usage.heapTotal,
                    external: usage.external,
                    timestamp: Date.now()
                };
            }
        } catch (error) {
            console.warn('Memory usage unavailable:', error);
        }
        
        return null;
    }
    
    startMonitoring(interval = 30000) { // Monitor every 30 seconds
        if (this.monitoringInterval) {
            this.stopMonitoring();
        }
        
        this.monitoringInterval = setInterval(() => {
            this.checkMemoryUsage();
        }, interval);
        
        console.log('Memory monitoring started');
    }
    
    stopMonitoring() {
        if (this.monitoringInterval) {
            clearInterval(this.monitoringInterval);
            this.monitoringInterval = null;
            console.log('Memory monitoring stopped');
        }
    }
    
    checkMemoryUsage() {
        const usage = this.getCurrentMemoryUsage();
        if (!usage) return;
        
        this.memoryStats = usage;
        
        // Check thresholds and trigger callbacks
        if (usage.heapUsed >= this.thresholds.critical) {
            this.triggerCallbacks('critical', usage);
        } else if (usage.heapUsed >= this.thresholds.cleanup) {
            this.triggerCallbacks('cleanup', usage);
        } else if (usage.heapUsed >= this.thresholds.warning) {
            this.triggerCallbacks('warning', usage);
        }
    }
    
    triggerCallbacks(type, usage) {
        const callbacks = this.callbacks[type] || [];
        callbacks.forEach(callback => {
            try {
                callback(usage);
            } catch (error) {
                console.error(`Memory callback error (${type}):`, error);
            }
        });
    }
    
    onMemoryWarning(callback) {
        this.callbacks.warning.push(callback);
    }
    
    onMemoryCritical(callback) {
        this.callbacks.critical.push(callback);
    }
    
    onMemoryCleanup(callback) {
        this.callbacks.cleanup.push(callback);
    }
    
    setupCleanupTriggers() {
        // Automatic cleanup on memory pressure
        this.onMemoryCleanup((usage) => {
            console.log('Memory cleanup triggered:', this.formatBytes(usage.heapUsed));
            this.performAutomaticCleanup();
        });
        
        this.onMemoryCritical((usage) => {
            console.warn('Critical memory usage:', this.formatBytes(usage.heapUsed));
            this.performAggressiveCleanup();
        });
    }
    
    performAutomaticCleanup() {
        try {
            // Trigger global cleanup if available
            if (typeof window !== 'undefined' && window.globalState) {
                window.globalState.forceCleanup();
            }
            
            // Clear any cached DOM references
            this.clearDOMCache();
            
            // Force garbage collection if available
            this.forceGarbageCollection();
            
            console.log('Automatic memory cleanup completed');
        } catch (error) {
            console.error('Automatic cleanup error:', error);
        }
    }
    
    performAggressiveCleanup() {
        try {
            // Perform automatic cleanup first
            this.performAutomaticCleanup();
            
            // Clear all possible caches
            this.clearAllCaches();
            
            // Multiple GC attempts
            for (let i = 0; i < 3; i++) {
                setTimeout(() => this.forceGarbageCollection(), i * 100);
            }
            
            console.log('Aggressive memory cleanup completed');
        } catch (error) {
            console.error('Aggressive cleanup error:', error);
        }
    }
    
    clearDOMCache() {
        if (typeof document !== 'undefined') {
            // Clear cached elements
            const cachedElements = document.querySelectorAll('[data-cached]');
            cachedElements.forEach(el => {
                el.removeAttribute('data-cached');
            });
            
            // Clear any temporary DOM references
            if (window._tempDOMRefs) {
                window._tempDOMRefs.clear();
            }
        }
    }
    
    clearAllCaches() {
        if (typeof window !== 'undefined') {
            // Clear window-level caches
            Object.keys(window).forEach(key => {
                if (key.startsWith('cache_') || key.startsWith('_cache')) {
                    try {
                        delete window[key];
                    } catch (e) {
                        // Ignore errors for non-configurable properties
                    }
                }
            });
        }
    }
    
    forceGarbageCollection() {
        try {
            if (typeof global !== 'undefined' && global.gc) {
                global.gc();
            } else if (typeof window !== 'undefined' && window.gc) {
                window.gc();
            }
        } catch (error) {
            // GC not available, ignore
        }
    }
    
    getMemoryStats() {
        return {
            current: this.memoryStats,
            thresholds: this.thresholds,
            formatted: {
                heapUsed: this.formatBytes(this.memoryStats.heapUsed),
                heapTotal: this.formatBytes(this.memoryStats.heapTotal),
                external: this.formatBytes(this.memoryStats.external)
            }
        };
    }
    
    formatBytes(bytes) {
        if (bytes === 0) return '0 Bytes';
        
        const k = 1024;
        const sizes = ['Bytes', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        
        return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
    }
    
    setThresholds(warning, cleanup, critical) {
        this.thresholds.warning = warning;
        this.thresholds.cleanup = cleanup;
        this.thresholds.critical = critical;
    }
    
    destroy() {
        this.stopMonitoring();
        this.callbacks = { warning: [], critical: [], cleanup: [] };
        console.log('Memory monitor destroyed');
    }
}

// Create singleton instance
const memoryMonitor = new MemoryMonitor();

export { memoryMonitor, MemoryMonitor };