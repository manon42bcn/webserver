/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServerResponseHandler.hpp                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/07 09:37:41 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/07 17:18:10 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#ifndef _WEBSERVER_RESPONSE_HANDLER_
#define _WEBSERVER_RESPONSE_HANDLER_
#include "webserver.hpp"
#include "http_enum_codes.hpp"
#include "Logger.hpp"
#include "ClientData.hpp"
#include <string>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <sys/wait.h>

#define RSP_NAME "HttpResponseHandler"
#define CGI_TIMEOUT 5000
#define UNUSED(x) (void)(x)
#define DEFAULT_RANGE_BYTES 65536

enum e_content_type {
	CT_UNKNOWN = 0,
	CT_FILE = 1,
	CT_JSON = 2
};

enum e_range_scenario {
	CR_INIT = 0,
	CR_RANGE = 1,
	CR_LAST = 2
};

struct s_content {
	bool                ranged;
	size_t              start;
	size_t              end;
	size_t              filesize;
	e_range_scenario    range_scenario;
	bool                status;
	std::string         content;
	std::string         mime;
	e_http_sts          http_status;
	std::string         header;
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
	s_multi_part(std::string fn, e_content_type dt, std::string d):
			filename(fn), data(d), data_type(dt) {};
};

class WsResponseHandler {
private:
	int                     	_fd;
	std::vector<s_multi_part>	_multi_content;
	std::string            		_content;
	std::vector<std::string>    _response_header;

protected:
	const LocationConfig*   	_location;
	const Logger*           	_log;
	ClientData*             	_client_data;
	s_request&              	_request;
	s_content                   _response_data;
	std::string                 _headers;

public:
	WsResponseHandler(const LocationConfig *location,
		              const Logger *log,
					  ClientData* client_data,
					  s_request& request,
					  int fd);

	virtual ~WsResponseHandler() = 0;
	virtual void get_file_content(int pid, int (&fd)[2]) = 0;
	virtual void get_file_content(std::string& path);
	virtual bool handle_request();
	virtual bool send_response(const std::string& body, const std::string& path);
	virtual bool handle_get();
	bool handle_post();
	bool handle_delete();
	std::string header(int code, size_t content_size, std::string mime);
	std::string default_plain_error();
	void get_post_content();
	void parse_multipart_data();
	void validate_payload();
	bool send_error_response();
	bool sender(const std::string& body);
	void turn_off_sanity(e_http_sts status, std::string detail);
// dirty implementation

};

#endif
