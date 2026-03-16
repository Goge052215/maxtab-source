import { dict } from './dic.js'

let SimpleInputMethod = {
    dict: {},
    // Add cache management
    _cacheSize: 0,
    _maxCacheSize: 500, // Limit cache size
    _lastCleanup: Date.now()
}

SimpleInputMethod.initDict = function() {
    SimpleInputMethod.dict.py2hz = dict;
    SimpleInputMethod.dict.py2hz2 = {};
    SimpleInputMethod.dict.py2hz2['i'] = 'i'; // i比较特殊，没有符合的汉字，所以特殊处理

    // Build cache more efficiently
    for (let key in SimpleInputMethod.dict.py2hz) {
        let ch = key[0];
        if (!SimpleInputMethod.dict.py2hz2[ch]) {
            SimpleInputMethod.dict.py2hz2[ch] = SimpleInputMethod.dict.py2hz[key];
            SimpleInputMethod._cacheSize++;
        }
    }
    
    // Setup periodic cleanup
    SimpleInputMethod._setupCleanup();
};

// Setup cleanup mechanism
SimpleInputMethod._setupCleanup = function() {
    // Every 5 minutes
    setInterval(() => {
        SimpleInputMethod._performCleanup();
    }, 300000);
};

// Perform cache cleanup
SimpleInputMethod._performCleanup = function() {
    const now = Date.now();
    
    // Only cleanup if cache is too large or enough time has passed
    if (SimpleInputMethod._cacheSize > SimpleInputMethod._maxCacheSize || (now - SimpleInputMethod._lastCleanup) > 600000) {
        // Clear less frequently used cache entries
        const keysToDelete = [];
        let count = 0;
        
        for (let key in SimpleInputMethod.dict.py2hz2) {
            if (key !== 'i' && count < Math.floor(SimpleInputMethod._cacheSize * 0.3)) {
                keysToDelete.push(key);
                count++;
            }
        }
        
        keysToDelete.forEach(key => {
            delete SimpleInputMethod.dict.py2hz2[key];
            SimpleInputMethod._cacheSize--;
        });
        
        SimpleInputMethod._lastCleanup = now;
    }
};

SimpleInputMethod.getSingleHanzi = function(pinyin){
    // Check cache first, then fallback to main dict
    let result = SimpleInputMethod.dict.py2hz2[pinyin];
    if (result) return result;
    
    result = SimpleInputMethod.dict.py2hz[pinyin];
    if (result && SimpleInputMethod._cacheSize < SimpleInputMethod._maxCacheSize) {
        // Cache frequently accessed items
        SimpleInputMethod.dict.py2hz2[pinyin] = result;
        SimpleInputMethod._cacheSize++;
    }
    
    return result || '';
}

SimpleInputMethod.getHanzi = function(pinyin) {
    let result = SimpleInputMethod.getSingleHanzi(pinyin);
    if (result) return [result.split(''), pinyin];

    let start = Math.min(pinyin.length, 6);

    for (let i = start; i >= 1; i--) {
        let str = pinyin.substr(0, i);
        let rs = SimpleInputMethod.getSingleHanzi(str);
        if (rs) return [rs.split(''), str];
    }

    return [[], '']; // 理论上一般不会出现这种情况
};

// Add cleanup method
SimpleInputMethod.cleanup = function() {
    try {
        // Clear all caches
        SimpleInputMethod.dict.py2hz2 = {};
        SimpleInputMethod._cacheSize = 0;
        
        // Keep only essential entry
        SimpleInputMethod.dict.py2hz2['i'] = 'i';
        SimpleInputMethod._cacheSize = 1;
    } catch (error) {
        // InputMethod cleanup error
    }
};

SimpleInputMethod.initDict();

export { SimpleInputMethod } //换成export default SimpleInputMethod;不能用
