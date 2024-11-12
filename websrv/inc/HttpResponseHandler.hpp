/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseHandler.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/09 22:15:33 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HTTP_RESPONSEHANDLER_HPP_
#define _HTTP_RESPONSEHANDLER_HPP_

#include "WebServerResponseHandler.hpp"
#include "WebserverCache.hpp"

#define RHB_NAME "HttpResponseHandler"

/**
 * @class HttpResponseHandler
 * @brief Manages HTTP responses, with support for caching static file content.
 *
 * The `HttpResponseHandler` class extends `WsResponseHandler` to handle HTTP responses,
 * utilizing a caching mechanism to optimize retrieval of static files.
 * It checks for content in the cache before loading from disk, reducing redundant
 * I/O operations and improving response times for frequently requested resources.
 */
class HttpResponseHandler : public WsResponseHandler {
	private:
		WebServerCache<CacheEntry>& _cache;
	public:
		HttpResponseHandler(const LocationConfig *location,
							const Logger *log,
							ClientData* client_data,
							s_request& request,
							int fd,
							WebServerCache<CacheEntry>& cache);
	void get_file_content(std::string& path);
	void get_file_content(int pid, int (&fd)[2]);
};

#endif
