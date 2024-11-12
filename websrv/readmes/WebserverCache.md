
# WebServerCache Template Class

## Overview
`WebServerCache` is a generic, templated class that manages a cache using an LRU (Least Recently Used) eviction policy. This class allows efficient storage and retrieval of data with a specified maximum capacity. When the cache reaches this capacity, the least recently used item is removed to make space for new entries.

### Key Features
- **Efficient Data Access**: Uses `std::map` and `std::list` for quick lookups and ordered access.
- **LRU Eviction Policy**: Automatically evicts the least recently used item when the cache capacity is reached.

## Template Parameter
- `T`: Type of the entries stored in the cache. This type must support copying, and if used with `put` operations, should have a `std::string`-based unique identifier: `url`.

## Public Interface

### Constructor
```cpp
explicit WebServerCache(size_t capacity);
```
- **Parameters**:
    - `capacity`: Maximum number of entries the cache can hold.

### Methods

#### `bool get(const std::string& key, T& entry)`
Retrieves an entry from the cache if it exists and marks it as recently used.
- **Parameters**:
    - `key`: The unique identifier for the cache entry.
    - `entry`: Reference where the retrieved cache entry is stored.
- **Returns**: `true` if the entry is found, `false` otherwise.

#### `void put(const std::string& key, const T& entry)`
Inserts or updates an entry in the cache. If the entry already exists, it is updated and marked as recently used.
- **Parameters**:
    - `key`: The unique identifier for the cache entry.
    - `entry`: The entry data to be cached.

## Private Members

### `_capacity` (size_t)
Holds the maximum capacity of the cache.

### `_entries` (std::list<T>)
Stores the cache entries in order of recency, with the most recent at the beginning.

### `_cache_map` (std::map<std::string, typename std::list<T>::iterator>)
Maps each entry’s key to its position in `_entries` for fast lookups and updates.

## Considerations
- Ensure `T` contains a unique key identifier compatible with `std::string`.
- Designed for high-performance caching but is not thread-safe.

