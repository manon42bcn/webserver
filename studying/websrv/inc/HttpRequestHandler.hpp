/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/07 09:37:41 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef _HTTPREQUESTHANDLER_HPP_
#define _HTTPREQUESTHANDLER_HPP_

// Includes
#include "webserver.hpp"
#include "http_enum_codes.hpp"
#include "ClientData.hpp"
#include "Logger.hpp"
#include "HttpResponseHandler.hpp"
#include "HttpCGIHandler.hpp"
#include "HttpRangeHandler.hpp"
// Libraries
#include <string>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
// Defines
#define RH_NAME "HttpRequestHandler"
#define BUFFER_REQUEST  2048
#define MAX_REQUEST     52428800 // 50mb -> it should be loaded by config...
#define URI_MAX         2048

typedef struct e_chunk {
	size_t      size;
	std::string chunk;
} t_chunk;

/**
 * @class HttpRequestHandler
 * @brief Handles and validates incoming HTTP requests, managing the request path, headers, and body.
 *
 * This class processes incoming HTTP requests by reading headers, validating method and path,
 * handling CGI requests, and creating an `s_request` structure for further handling.
 * It performs multiple validation steps and determines how to handle the request based on
 * the server configuration and HTTP method.
 *
 * @details
 * - The constructor initializes the handler with a logger and client data, and performs a series of
 *   validation steps to ensure the request is properly formed.
 * - This class delegates the response generation to `HttpResponseHandler` after building a request wrapper.
 * - Supports methods like GET, POST, DELETE, and handles requests for CGI scripts.
 * - Logs each major step to assist with debugging and request tracing.
 *
 * @note This class expects a valid `Logger` pointer, and throws `Logger::NoLoggerPointer` if none is provided.
 *
 * ### Private Attributes
 * - **Configuration & Client Information**:
 *   - `_config`: Reference to the server's configuration, containing root paths and other settings.
 *   - `_log`: Pointer to the logger instance for recording actions and errors.
 *   - `_client_data`: Pointer to the client’s data, including the server instance.
 *   - `_location`: Pointer to the configuration for the specific location being requested.
 *
 * - **Request Details**:
 *   - `_request`, `_header`, `_body`: Strings holding the full request, header, and body content.
 *   - `_method`, `_path`, `_normalized_path`: Parsed HTTP method and paths.
 *   - `_content_type`, `_content_length`, `_boundary`: Content-specific headers for processing body data.
 *
 * - **Request Status**:
 *   - `_sanity`, `_status`: Flags to indicate the validity and status of the request.
 *   - `_cgi`, `_script`, `_path_info`: Flags and data specific to CGI request handling.
 *
 * ### Public Methods
 * - `HttpRequestHandler(const Logger* log, ClientData* client_data)`: Constructor that initializes and validates the request.
 * - `~HttpRequestHandler()`: Destructor to clean up and release resources, logging the cleanup.
 *
 * ### Private Methods
 * - `read_request_header()`, `parse_method_and_path()`, `parse_header()`: Handle header reading and parsing.
 * - `normalize_request_path()`, `cgi_normalize_path()`: Normalize paths and handle CGI paths if applicable.
 * - `validate_request()`, `turn_off_sanity()`: Validate request content and disable processing if invalid.
 *
 */
class HttpRequestHandler {
	private:
	    typedef void (HttpRequestHandler::*validate_step)( );
		const ServerConfig&     _config;
		const Logger*           _log;
		ClientData*             _client_data;
		const LocationConfig*   _location;
		int                     _fd;
		size_t 					_max_request;
//	    request
	    std::string             _request;
	    std::string             _header;
	    std::string             _body;
//	    parsed
	    e_methods               _method;
		std::string 			_content_type;
		size_t                  _content_length;
		std::string             _path;
	    std::string             _normalized_path;
		std::string 			_query_string;
		std::string 			_path_info;
		e_path_type				_path_type;
	    std::string             _boundary;
	    e_access                _access;
	    bool                    _sanity;
	    e_http_sts              _status;
		bool					_cgi;
		std::string 			_script;
		bool                    _chunks;
		std::map<std::string, std::string>  _all_headers;
		std::string             _range;
		bool					_active;
		int                     _factory;

		void read_request_header();
		void parse_method_and_path();
		void parse_path_type();
	    void parse_header();
	    void load_header_data();
	    void load_content();
		void load_content_normal();
		void load_content_chunks();
		bool parse_chunks(std::string& chunk_data, long& chunk_size);
	    void validate_request();
	    void get_location_config();
		void handle_request();
		void normalize_request_path();
		void cgi_normalize_path();
	    void turn_off_sanity(e_http_sts status, std::string detail);

	public:
		HttpRequestHandler(const Logger* log, ClientData* client_data);
	    ~HttpRequestHandler();

};


#endif

