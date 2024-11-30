/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/30 17:45:30 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef _HTTPREQUESTHANDLER_HPP_
#define _HTTPREQUESTHANDLER_HPP_
// Includes
#include "webserver.hpp"
#include "http_enum_codes.hpp"
#include "ClientData.hpp"
#include "Logger.hpp"
#include "WebserverCache.hpp"
#include "HttpRequestHandler.hpp"
#include "HttpResponseHandler.hpp"
#include "HttpCGIHandler.hpp"
#include "HttpRangeHandler.hpp"
#include "HttpMultipartHandler.hpp"
#include "HttpAutoIndex.hpp"
#include "WebserverCache.hpp"
// Libraries
#include <string>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <sstream>
// Defines
#define RH_NAME "HttpRequestHandler"
#define BUFFER_REQUEST  2048
#define URI_MAX         2048
#define MAX_HEADER      16384

typedef struct e_chunk {
	size_t      size;
	std::string chunk;
} t_chunk;


/**
 * @brief Handles HTTP requests by parsing, validating, and dispatching them to appropriate response handlers.
 *
 * The `HttpRequestHandler` class is responsible for managing the complete lifecycle of an HTTP request,
 * including:
 * - Parsing request headers, method, path, and any additional data such as query parameters and multipart boundaries.
 * - Validating the request's structure and content.
 * - Determining the appropriate handler based on request type (e.g., standard, CGI, ranged, or multipart).
 * - Generating an error response if the request is invalid or malformed.
 *
 * The class utilizes a series of private validation and parsing steps, executed in sequence through
 * `request_workflow`. If any step detects an invalid state, it halts the process and sends an error response.
 *
 * ## Attributes
 * - `_config`: Reference to the server configuration.
 * - `_log`: Pointer to the logging utility for logging events and errors.
 * - `_client_data`: Manages client-specific information, including connection state and timing.
 * - `_cache`: Pointer to the server cache to leverage cached responses.
 * - `_location`: Configuration for the specific URL location being requested.
 * - `_fd`: File descriptor associated with the client request.
 * - `_max_request`: Maximum allowed size for request data.
 * - `_request`: Raw request data.
 * - `_factory`: Determines the handler type (standard, CGI, range, etc.).
 * - `_request_data`: Stores parsed request details, including headers, method, path, and body.
 *
 * The class supports CGI requests, ranged requests, multipart uploads, and standard HTTP methods,
 * using helper classes (`HttpResponseHandler`, `HttpCGIHandler`, `HttpRangeHandler`, and `HttpMultipartHandler`)
 * to handle each type.
 *
 * @exception WebServerException Thrown for issues with cache, logging, or invalid configurations.
 * @see HttpResponseHandler, HttpCGIHandler, HttpRangeHandler, HttpMultipartHandler
 */
class HttpRequestHandler {
	private:
	    typedef void (HttpRequestHandler::*validate_step)( );
		ServerConfig*                   _host_config;
		ServerConfig&                   _config;
		const Logger*                   _log;
		ClientData*                     _client_data;
		const LocationConfig*           _location;
		int                             _fd;
		size_t 					        _max_request;
		std::string                     _request;
		s_request&                      _request_data;
		CacheRequest                    _cache_data;
		WebServerCache<CacheRequest>*   _cache;

		void read_request_header();
		void parse_header();
		void parse_method_and_path();
		void parse_path_type();
		void load_header_data();
		void load_host_config();
		void solver_resource();
		void resolve_relative_path();
		void get_location_config();
		void cgi_normalize_path();
		void normalize_request_path();
		void load_content();
		void load_content_normal();
		void load_content_chunks();
		bool parse_chunks(std::string& chunk_data, long& chunk_size);
		void validate_request();
	    void turn_off_sanity(e_http_sts status, std::string detail);

	public:
		HttpRequestHandler(const Logger* log,
						   ClientData* client_data);
	    ~HttpRequestHandler();
		void request_workflow();
		void handle_request();
};

#endif
