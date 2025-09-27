import { dict } from './dic.js'

let SimpleInputMethod = {
    dict: {},
    // Add cache management
    _cacheSize: 0,
    _maxCacheSize: 500, // Limit cache size
    _lastCleanup: Date.now()
}

SimpleInputMethod.initDict = function() {
    this.dict.py2hz = dict;
    this.dict.py2hz2 = {};
    this.dict.py2hz2['i'] = 'i'; // i比较特殊，没有符合的汉字，所以特殊处理

    // Build cache more efficiently
    for (let key in this.dict.py2hz) {
        let ch = key[0];
        if (!this.dict.py2hz2[ch]) {
            this.dict.py2hz2[ch] = this.dict.py2hz[key];
            this._cacheSize++;
        }
    }
    
    // Setup periodic cleanup
    this._setupCleanup();
};

// Setup cleanup mechanism
SimpleInputMethod._setupCleanup = function() {
    // Every 5 minutes
    setInterval(() => {
        this._performCleanup();
    }, 300000);
};

// Perform cache cleanup
SimpleInputMethod._performCleanup = function() {
    const now = Date.now();
    
    // Only cleanup if cache is too large or enough time has passed
    if (this._cacheSize > this._maxCacheSize || (now - this._lastCleanup) > 600000) {
        // Clear less frequently used cache entries
        const keysToDelete = [];
        let count = 0;
        
        for (let key in this.dict.py2hz2) {
            if (key !== 'i' && count < Math.floor(this._cacheSize * 0.3)) {
                keysToDelete.push(key);
                count++;
            }
        }
        
        keysToDelete.forEach(key => {
            delete this.dict.py2hz2[key];
            this._cacheSize--;
        });
        
        this._lastCleanup = now;
        console.log(`InputMethod cache cleaned: ${keysToDelete.length} entries removed`);
    }
};

SimpleInputMethod.getSingleHanzi = function(pinyin){
    // Check cache first, then fallback to main dict
    let result = this.dict.py2hz2[pinyin];
    if (result) return result;
    
    result = this.dict.py2hz[pinyin];
    if (result && this._cacheSize < this._maxCacheSize) {
        // Cache frequently accessed items
        this.dict.py2hz2[pinyin] = result;
        this._cacheSize++;
    }
    
    return result || '';
}

SimpleInputMethod.getHanzi = function(pinyin) {
    let result = this.getSingleHanzi(pinyin);
    if (result) return [result.split(''), pinyin];

    let temp = '';
    let start = Math.min(pinyin.length, 6);

    for (let i = start; i >= 1; i--) {
        let str = pinyin.substr(0, i);
        let rs = this.getSingleHanzi(str);
        if (rs) return [rs.split(''), str];
    }

    return [[], '']; // 理论上一般不会出现这种情况
};

// Add cleanup method
SimpleInputMethod.cleanup = function() {
    try {
        // Clear all caches
        this.dict.py2hz2 = {};
        this._cacheSize = 0;
        
        // Keep only essential entry
        this.dict.py2hz2['i'] = 'i';
        this._cacheSize = 1;
        
        console.log('InputMethod cache cleared');
    } catch (error) {
        console.error('InputMethod cleanup error:', error);
    }
};

SimpleInputMethod.initDict();

export { SimpleInputMethod } //换成export default SimpleInputMethod;不能用