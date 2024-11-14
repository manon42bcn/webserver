/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebserverCache.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 09:53:58 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/13 00:44:47 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef _WEBSERVER_CACHE_HPP_
#define _WEBSERVER_CACHE_HPP_

#include <map>
#include <list>
#include <string>


/**
 * @brief A simple, in-memory Least Recently Used (LRU) cache.
 *
 * The `WebServerCache` class is designed to manage a fixed-size cache for
 * storing frequently accessed items in a web server context. It utilizes a
 * Least Recently Used (LRU) eviction policy, where the least recently accessed
 * entry is removed when the cache reaches its maximum capacity.
 *
 * @tparam T The data type of the entries stored in the cache.
 *           The data type `T` should have a `std::string` member named `url`
 *           or a similar identifier that can be used to uniquely identify cache entries.
 */
template <typename T>
class WebServerCache {
private:
	size_t                                                  _capacity;
	std::list<T>                                            _entries;
	std::map<std::string, typename std::list<T>::iterator>  _cache_map;

public:
	/**
     * @brief Constructs a cache with a specified maximum capacity.
     *
     * @param capacity The maximum number of entries the cache can hold.
     */
	explicit WebServerCache(size_t capacity) : _capacity(capacity) {}

	/**
     * @brief Retrieves an entry from the cache.
     *
     * This method searches the cache for an entry with the specified key.
     * If the entry is found, it is moved to the front of the LRU list.
     *
     * @param key The unique key identifying the cache entry.
     * @param entry Reference to an object where the retrieved entry will be stored.
     *
     * @return `true` if the entry was found and `entry` was updated; otherwise, `false`.
     */
	bool get(const std::string& key, T& entry) {
		typename std::map<std::string, typename std::list<T>::iterator>::iterator it = _cache_map.find(key);
		if (it == _cache_map.end()) {
			return false;
		}

		_entries.splice(_entries.begin(), _entries, it->second);
		entry = *(it->second);
		return true;
	}

	/**
     * @brief Adds or updates an entry in the cache.
     *
     * If an entry with the specified key already exists, it is updated and moved
     * to the front of the LRU list. If the cache is at capacity, the least
     * recently used entry is removed to make space for the new entry.
     *
     * @param key The unique key identifying the cache entry.
     * @param entry The entry to be added or updated in the cache.
     */
	void put(const std::string& key, const T& entry) {
		typename std::map<std::string, typename std::list<T>::iterator>::iterator it = _cache_map.find(key);

		if (it != _cache_map.end()) {
			_entries.splice(_entries.begin(), _entries, it->second);
			*(it->second) = entry;
			return;
		}

		if (_entries.size() >= _capacity) {
			std::string lru_key = _entries.back().url;
			_cache_map.erase(lru_key);
			_entries.pop_back();
		}

		_entries.push_front(entry);
		_cache_map[key] = _entries.begin();
	}
	/**
	* @brief Removes a specific entry from the cache by its key.
	* This function searches for the entry with the specified key in the cache. If found, it removes the entry from both
	* the list (which maintains the order of usage) and the map (which provides quick access by key).
	*
	* @param key The key associated with the cache entry to be removed.
	*
	*@note The cache is updated to remove the entry, making space for new entries if necessary.
	*/
	void remove(const std::string& key) {
		typename std::map<std::string, typename std::list<T>::iterator>::iterator it = _cache_map.find(key);

		if (it != _cache_map.end()) {
			_entries.erase(it->second);
			_cache_map.erase(it);
		}
	}
};

#endif
