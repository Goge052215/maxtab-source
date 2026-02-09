class Node {
  constructor(key, value) {
    this.key = key;
    this.value = value;
    this.prev = null;
    this.next = null;
  }
}

class LRUCache {
  constructor(maxSize) {
    this.maxSize = maxSize;
    this.cache = new Map();
    this.head = new Node(null, null);
    this.tail = new Node(null, null);
    this.head.next = this.tail;
    this.tail.prev = this.head;
  }

  _remove(node) {
    node.prev.next = node.next;
    node.next.prev = node.prev;
  }

  _add(node) {
    node.next = this.head.next;
    node.prev = this.head;
    this.head.next.prev = node;
    this.head.next = node;
  }

  get(key) {
    if (!this.cache.has(key)) {
      return null;
    }

    const node = this.cache.get(key);
    this._remove(node);
    this._add(node);
    return node.value;
  }

  set(key, value) {
    if (this.maxSize <= 0) {
      return;
    }

    if (this.cache.has(key)) {
      const existingNode = this.cache.get(key);
      existingNode.value = value;
      this._remove(existingNode);
      this._add(existingNode);
      return;
    }

    if (this.cache.size >= this.maxSize) {
      const oldestNode = this.tail.prev;
      this._remove(oldestNode);
      this.cache.delete(oldestNode.key);
    }

    const newNode = new Node(key, value);
    this.cache.set(key, newNode);
    this._add(newNode);
  }

  has(key) {
    return this.cache.has(key);
  }

  clear() {
    this.cache.clear();
    this.head.next = this.tail;
    this.tail.prev = this.head;
  }

  get size() {
    return this.cache.size;
  }
}

export default LRUCache;
