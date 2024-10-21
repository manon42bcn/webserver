/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/21 13:15:55 by mporras-         ###   ########.fr       */
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
#define BUFFER_REQUEST  1024
#define MAX_REQUEST     52428800 // 50mb -> it should be loaded by config...
#define URI_MAX         2048

struct s_request {
	std::string header;
	bool        body_included;
	size_t 		body_size;
	std::string body;
};

enum e_rqs_state {
	ST_INIT = 0,
	ST_LOAD_LOCATION = 1,
	ST_READING = 2,
	ST_PARSING = 3,
	ST_LOAD_FILE = 4,
	ST_ERROR_READING = 5,
	ST_ERROR_PROCESS = 6,
	ST_METHOD_PATH = 7
};

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
		const ServerConfig&     _config;
		const Logger*           _log;
		ClientData&             _client_data;
		const LocationConfig*   _location;
		bool                    _state;
		e_access                _access;
		e_http_sts              _http_status;
		e_methods               _method;
		int                     _fd;
		size_t 					_max_request;

		// Init request handler
		std::string read_http_request();
		std::string parse_request_and_method(const std::string& request);
	    s_request parse_request(const std::string& request);
	    void validate_parsed_request(s_request& request);
		std::string get_header_value(std::string header, std::string key);
	    void get_location_config(const std::string& path);
		bool handle_request(const std::string& path);
		s_path normalize_request_path(const std::string& requested_path) const;

	public:
		HttpRequestHandler(const Logger* log, ClientData& client_data);
		//	Temporal Method to debug responses for each method
//		void send_detailed_response(std::string requested_path);

};


#endif

