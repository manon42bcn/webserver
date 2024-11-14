/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseHandler.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/13 01:11:08 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponseHandler.hpp"

/**
 * @brief Constructs an `HttpResponseHandler` instance for handling HTTP responses.
 *
 * Initializes the response handler with configuration, logging, client data,
 * and the server cache, setting up the necessary components for managing
 * static responses.
 *
 * @param location Pointer to the `LocationConfig` containing the configuration details
 * 				   for the server location.
 * @param log Pointer to the `Logger` instance to record request and response details.
 * @param client_data Pointer to `ClientData` associated with the client connection.
 * @param request Reference to the `s_request` structure containing request details.
 * @param fd File descriptor for the client connection.
 * @param cache Pointer to the `WebServerCache` for handling cached responses and
 * 			    resources.
 */
HttpResponseHandler::HttpResponseHandler(const LocationConfig *location,
										 const Logger *log,
										 ClientData *client_data,
										 s_request &request,
                                         int fd) :
										 WsResponseHandler(location, log,
														   client_data, request,
														   fd),
									     _cache(client_data->get_server()->get_cache()) {
	_log->log_debug( RHB_NAME,
			  "Static Response Handler Init.");
}

/**
 * @brief Retrieves the content of a file, utilizing the cache if available.
 *
 * Attempts to retrieve the file content from the cache. If the content is not
 * cached, it loads the file content from disk using the base `WsResponseHandler`
 * method. If the content is successfully loaded, it is then stored in the cache.
 *
 * @param path Path to the file whose content is to be retrieved.
 */
void HttpResponseHandler::get_file_content(std::string &path) {
	CacheEntry data;
	if (!_cache.get(path, data)) {
		WsResponseHandler::get_file_content(path);
		if (_request.sanity) {
			_cache.put(path, CacheEntry(path, _response_data.content));
		}
	} else {
		struct stat buffer;
		if (stat(path.c_str(), &buffer) == 0) {
			_response_data.content = data.content;
			_response_data.status = true;
		} else {
			_cache.remove(path);
			turn_off_sanity(HTTP_NOT_FOUND,
							"File is not found.");
		}
	}
}

/**
 * @brief Placeholder for retrieving file content using process ID and file descriptor array.
 *
 * @note Unused function
 */
void HttpResponseHandler::get_file_content(int pid, int (&fd)[2]) {
	UNUSED(pid);
	UNUSED(fd);
}
