/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/17 15:16:16 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HTTPREQUESTHANDLER_HPP_
#define _HTTPREQUESTHANDLER_HPP_

#include "webserver.hpp"
#include "http_enum_codes.hpp"
#include "ClientData.hpp"
#include "Logger.hpp"
#include <string>

#define BUFFER_REQUEST  1024
#define MAX_REQUEST     8192
#define URI_MAX         2048

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


struct s_path {
	e_http_sts  code;
	std::string path;
	s_path(e_http_sts c, const std::string p) : code(c), path(p) {}
};

struct s_content {
	std::string content;
	size_t      size;
	s_content(std::string c, size_t s): content(c), size(s) {};
};

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
		const std::string       _module;
		bool                     _state;
		e_access                _access;
		e_http_sts              _http_status;
		e_methods               _method;
		int                     _fd;

		// Init request handler
		std::string read_http_request();
		std::string parse_request_and_method(const std::string& request);
		void get_location_config(const std::string& path);
		// Entrypoint to workflow
		bool handle_request(const std::string& path);
		s_path normalize_request_path(const std::string& requested_path) const;
		std::string default_plain_error();
		bool send_error_response();
	    bool sender(const std::string& body, const std::string& path);
		std::string get_file_content(const std::string& path);
		static std::string response_header(int code, size_t content_size, std::string mime);
		// Handle different methods
		void handle_get(const std::string& requested_path);
		void handle_post(const std::string& requested_path);
		void handle_delete(const std::string& requested_path);
		// Additional helper methods to handle file serving and MIME types (optional to include here)
		std::map<std::string, std::string> create_mime_types();
		std::string get_mime_type(const std::string& path);
	public:
		HttpRequestHandler(const Logger* log, ClientData& client_data);
		//	Temporal Method to debug responses for each method
		void send_detailed_response(std::string requested_path);


};


#endif

