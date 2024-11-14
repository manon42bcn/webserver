/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServerResponseHandler.hpp                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/07 09:37:41 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/14 19:02:54 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#ifndef _WEBSERVER_RESPONSE_HANDLER_
#define _WEBSERVER_RESPONSE_HANDLER_
#include "webserver.hpp"
#include "http_enum_codes.hpp"
#include "Logger.hpp"
#include "ClientData.hpp"
#include "ws_permissions_bitwise.hpp"
#include <string>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define RSP_NAME "HttpResponseHandler"
#define UNUSED(x) (void)(x)
#define DEFAULT_RANGE_BYTES 65536

/**
 * @brief Defines types of content handled in HTTP responses.
 */
enum e_content_type {
	CT_UNKNOWN = 0,
	CT_FILE = 1,
	CT_JSON = 2
};

/**
 * @brief Describes different scenarios for handling byte range requests.
 */
enum e_range_scenario {
	CR_INIT = 0,
	CR_RANGE = 1,
	CR_LAST = 2
};


/**
 * @brief Holds data associated with HTTP response content.
 *
 * This structure stores various attributes related to the response content,
 * including range data, MIME type, content length, and response status.
 */
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
	s_content(): ranged(false), start(0), end(0), filesize(0),
	             range_scenario(CR_INIT),status(false), content(""),
	             mime(""), http_status(HTTP_I_AM_A_TEAPOT), header("") {};
};


/**
 * @class WsResponseHandler
 * @brief Base class for handling HTTP responses in a web server.
 *
 * The `WsResponseHandler` class manages the processing of HTTP responses,
 * supporting GET, POST, and DELETE operations. It handles file reading,
 * content validation, response headers, error responses, and client interactions.
 * Derived classes can implement additional logic, including specialized response
 * handling such as for multipart or CGI responses.
 */
class WsResponseHandler {
	private:
		int                     	_fd;
		std::string            		_content;
		std::vector<std::string>    _response_header;

	protected:
		const LocationConfig*   	_location;
		const Logger*           	_log;
		ClientData*             	_client_data;
		s_request&              	_request;
		s_content                   _response_data;
		std::string                 _headers;

		virtual bool handle_get();
		virtual bool handle_post();
		bool handle_delete();
		virtual bool validate_payload();
		virtual void get_file_content(int pid, int (&fd)[2]) = 0;
		virtual void get_file_content(std::string& path);
		bool save_file(const std::string& save_path, const std::string& content);
		virtual std::string header(int code, size_t content_size, std::string mime);
		virtual bool send_response(const std::string& body, const std::string& path);
		bool sender(const std::string& body);
		std::string default_plain_error();
		void turn_off_sanity(e_http_sts status, std::string detail);
	public:

		WsResponseHandler(const LocationConfig *location,
			              const Logger *log,
						  ClientData* client_data,
						  s_request& request,
						  int fd);
		virtual ~WsResponseHandler();
		virtual bool handle_request();
		bool send_error_response();
};

#endif
