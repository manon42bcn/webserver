/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebserverCache.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 09:53:58 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/08 14:12:12 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
# ifndef _WEBSERVER_CACHE_HPP_
#define _WEBSERVER_CACHE_HPP_

#include <map>
#include <list>
#include <string>

struct CacheEntry {
	std::string url;
	std::string content;
	int access_count;

	CacheEntry(const std::string &u, const std::string &c)
			: url(u), content(c), access_count(1) {}
};

class WebServerCache {
	private:
		size_t _capacity;
//		int _min_access_count;
		std::list<CacheEntry> _entries;
		std::map<std::string, std::list<CacheEntry>::iterator> _cache_map;
	public:
		WebServerCache(size_t capacity) : _capacity(capacity) {};
		bool get(const std::string &url, std::string &content);
		void put(const std::string &url, const std::string &content);
};

#endif
