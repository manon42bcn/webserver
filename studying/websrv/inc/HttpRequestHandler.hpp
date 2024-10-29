/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/29 11:00:34 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HTTPREQUESTHANDLER_HPP_
#define _HTTPREQUESTHANDLER_HPP_

#include "webserver.hpp"
#include "http_enum_codes.hpp"
#include "ClientData.hpp"
#include "Logger.hpp"
#include "HttpResponseHandler.hpp"
#include <string>

#define RH_NAME "HttpRequestHandler"
#define BUFFER_REQUEST  2048
#define MAX_REQUEST     52428800 // 50mb -> it should be loaded by config...
#define URI_MAX         2048

typedef enum s_is_file {
	DIR_IS = 0,
	FILE_IS = 1,
	NONE_IS = 3
} t_is_file;

/**
 * @brief Class to encapsulates the logic of get a request, process it and send a response/error
 *
 */
// TODO: state and access are wip.. just explore differents ways to use them
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
		std::string 			_content_type;
	    size_t                  _content_leght;
//	    parsed
	    e_methods               _method;
	    std::string             _path;
	    std::string             _normalized_path;
		std::string 			_query_encoded;
		e_path_type				_path_type;
	    std::string             _boundary;
	    e_access                _access;
	    bool                    _sanity;
	    e_http_sts              _status;
		bool					_cgi;

		// Init request handler
		void read_request_header();
		void parse_method_and_path();
		void parse_path_type();
	    void parse_header();
		bool is_cgi(const std::string& filename);
	    void load_header_data();
	    void load_content();
	    void validate_request();
//		void parse_content_type();
	    void get_location_config();
		void handle_request();
		void normalize_request_path();
//		void validate_post_path();
	    void turn_off_sanity(e_http_sts status, std::string detail);

	public:
		HttpRequestHandler(const Logger* log, ClientData* client_data);
	    ~HttpRequestHandler();
		//	Temporal Method to debug responses for each method
//		void send_detailed_response(std::string requested_path);

};


#endif

