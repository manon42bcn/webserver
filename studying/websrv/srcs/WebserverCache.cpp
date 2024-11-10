/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebserverCache.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 09:53:58 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/07 17:17:59 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebserverCache.hpp"

bool WebServerCache::get(const std::string &url, std::string &content) {
	std::map<std::string, std::list<CacheEntry>::iterator>::iterator it = _cache_map.find(url);
	if (it == _cache_map.end()) {
		return (false);
	}

	_entries.splice(_entries.begin(), _entries, it->second);
	content = it->second->content;
	return (true);
}

void WebServerCache::put(const std::string &url, const std::string &content) {
	std::map<std::string, std::list<CacheEntry>::iterator>::iterator it = _cache_map.find(url);

	if (it != _cache_map.end()) {
		_entries.splice(_entries.begin(), _entries, it->second);
		it->second->content = content;
		return;
	}

	if (_entries.size() >= _capacity) {
		std::string lru_url = _entries.back().url;
		_cache_map.erase(lru_url);
		_entries.pop_back();
	}

	_entries.push_front(CacheEntry(url, content));
	_cache_map[url] = _entries.begin();
}
