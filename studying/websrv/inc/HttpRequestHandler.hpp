/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/10 01:10:40 by mporras-         ###   ########.fr       */
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
#include "HttpMultipartHandler.hpp"
#include "WebserverCache.hpp"
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
#define MAX_REQUEST     52428800
#define URI_MAX         2048

typedef struct e_chunk {
	size_t      size;
	std::string chunk;
} t_chunk;

class HttpRequestHandler {
	private:
	    typedef void (HttpRequestHandler::*validate_step)( );
		const ServerConfig&     _config;
		const Logger*           _log;
		ClientData*             _client_data;
		WebServerCache*			_cache;
		const LocationConfig*   _location;
		int                     _fd;
		size_t 					_max_request;
	    std::string             _request;
		int                     _factory;
		s_request               _request_data;

		void read_request_header();
		void parse_header();
		void parse_method_and_path();
		void parse_path_type();
		void load_header_data();
		void get_location_config();
		void cgi_normalize_path();
		void normalize_request_path();
		void load_content();
		void load_content_normal();
		void load_content_chunks();
		bool parse_chunks(std::string& chunk_data, long& chunk_size);
		void validate_request();
		void handle_request();
	    void turn_off_sanity(e_http_sts status, std::string detail);

	public:
		HttpRequestHandler(const Logger* log,
						   ClientData* client_data,
						   WebServerCache* cache);
	    ~HttpRequestHandler();
		void request_workflow();
};


#endif

