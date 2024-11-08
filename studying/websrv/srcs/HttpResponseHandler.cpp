/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseHandler.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/08 14:09:30 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponseHandler.hpp"

HttpResponseHandler::~HttpResponseHandler() {}

HttpResponseHandler::HttpResponseHandler(const LocationConfig *location,
										 const Logger *log,
										 ClientData *client_data,
										 s_request &request,
                                         int fd,
										 WebServerCache* cache) :
										 WsResponseHandler(location, log,
														   client_data, request,
														   fd),
									     _cache(cache) {
	_log->log(LOG_DEBUG, RHB_NAME,
			  "Static Response Handler Init.");
}

void HttpResponseHandler::get_file_content(std::string &path) {
	if (!_cache->get(path, _response_data.content)) {
		WsResponseHandler::get_file_content(path);
		if (_request.sanity) {
			_cache->put(path, _response_data.content);
		}
	}
}

void HttpResponseHandler::get_file_content(int pid, int (&fd)[2]) {
	UNUSED(pid);
	UNUSED(fd);
}
