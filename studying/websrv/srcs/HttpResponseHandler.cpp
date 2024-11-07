/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseHandler.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/06 10:14:48 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponseHandler.hpp"

HttpResponseHandler::~HttpResponseHandler() {}

HttpResponseHandler::HttpResponseHandler(const LocationConfig *location,
										 const Logger *log,
										 ClientData *client_data,
										 s_request &request,
                                         int fd) :
										 WsResponseHandler(location, log,
														   client_data, request,
														   fd) {
	_log->log(LOG_DEBUG, RHB_NAME,
			  "Static Response Handler Init.");
}

void HttpResponseHandler::get_file_content(int pid, int (&fd)[2]) {
	UNUSED(pid);
	UNUSED(fd);
}
