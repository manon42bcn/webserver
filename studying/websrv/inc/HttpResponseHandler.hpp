/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseHandler.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/14 11:33:32 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSEHANDLER_HPP
#define HTTPRESPONSEHANDLER_HPP

#include "webserver.hpp"
#include "http_enum_codes.hpp"
#include "Logger.hpp"
#include "ClientData.hpp"
#include <string>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sys/socket.h>

#define RSP_NAME "HttpResponseHandler"

/**
 * @brief Handle HTTP responses
 *
 * Encapsulate methods to handle HTTP responses.
 * Handle state codes, content and Headers for HTTP responses.
 */
class HttpResponseHandler {
private:
	int                     _fd;
	e_http_sts              _http_status;
	const LocationConfig*   _location;
	const Logger*           _log;
	e_methods               _method;
	s_path&                 _resource;

public:
	HttpResponseHandler(int fd, e_http_sts status, const LocationConfig *location,
	                    const Logger *log, e_methods method, s_path& path);
	bool handle_get();
	std::string header(int code, size_t content_size, std::string mime);
	std::string default_plain_error();
	s_content get_file_content(const std::string& path);
	bool send_error_response();
	bool sender(const std::string& body, const std::string& path);
	bool handle_request();
};

#endif
