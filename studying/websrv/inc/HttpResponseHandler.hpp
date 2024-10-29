/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseHandler.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/28 14:50:35 by mporras-         ###   ########.fr       */
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

enum e_content_type {
	CT_UNKNOWN = 0,
	CT_FILE = 1,
	CT_JSON = 2
};

struct s_multi_part {
	std::string 	disposition;
	std::string 	name;
	std::string 	filename;
	std::string 	type;
	std::string 	data;
	e_content_type	data_type;
	e_http_sts      status;
	s_multi_part(std::string di, std::string n,
				 std::string fn, std::string t,
				 std::string d): disposition(di), name(n),
				 filename(fn), type(t), data(d), data_type(CT_FILE),
                 status(HTTP_CREATED) {};
};

/**
 * @brief Handle HTTP responses
 *
 * Encapsulate methods to handle HTTP responses.
 * Handle state codes, content and Headers for HTTP responses.
 */
class HttpResponseHandler {
private:
	int                     	_fd;
	const LocationConfig*   	_location;
	const Logger*           	_log;
	ClientData*             	_client_data;
	std::vector<s_multi_part>	_multi_content;
	std::string            		_content;
	s_request&              	_request;

public:
	HttpResponseHandler(const LocationConfig *location,
	                    const Logger *log,
						ClientData* client_data,
	                    s_request& request,
	                    int fd);
	bool handle_get();
	bool handle_post();
	bool handle_delete();
	std::string header(int code, size_t content_size, std::string mime);
	std::string default_plain_error();
	s_content get_file_content(std::string& path);
	void get_post_content();
	void parse_multipart_data();
	void validate_payload();
	bool send_error_response();
	bool sender(const std::string& body, const std::string& path);
	bool handle_request();
	void turn_off_sanity(e_http_sts status, std::string detail);
	std::vector<char *> cgi_environment ();
};

#endif
