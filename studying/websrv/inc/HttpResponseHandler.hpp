/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseHandler.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/24 07:25:06 by mporras-         ###   ########.fr       */
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
	const LocationConfig*   _location;
	const Logger*           _log;
	std::string             _content;
	s_request&              _request;

public:
	HttpResponseHandler(const LocationConfig *location,
	                    const Logger *log,
	                    s_request& request,
	                    int fd);
	bool handle_get();
	bool handle_post();
	std::string header(int code, size_t content_size, std::string mime);
	std::string default_plain_error();
	s_content get_file_content(std::string& path);
	void get_post_content();
	bool send_error_response();
	bool sender(const std::string& body, const std::string& path);
	bool handle_request();
	void turn_off_sanity(e_http_sts status, std::string detail);
};

#endif
