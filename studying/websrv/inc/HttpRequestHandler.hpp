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

typedef int t_code;
typedef struct s_path {
	t_code      code;
	std::string path;
	s_path(t_code c, const std::string p) : code(c), path(p) {}
} t_path;

/**
 * @brief Class to encapsulates the logic of get a request, process it and send a response/error
 *
 */
// TODO: state and access are wip.. just explore differents ways to use them
class HttpRequestHandler {
	private:
		int                     _client_socket;
		const ServerConfig&     _config;
		const Logger*           _log;
		ClientData&             _client_data;
		const LocationConfig*   _location;
		const std::string       _module;
		int                     _state;
		e_access                _access;
		e_http_sts              _http_status;
		e_methods               _method;

		// Init request handler
		std::string read_http_request();
		std::string parse_request_and_method(const std::string& request);
		void get_location_config(const std::string& path);
		// Entrypoint to workflow
		bool handle_request(const std::string& path);
		s_path normalize_request_path(const std::string& requested_path) const;
		std::string default_plain_error();
		bool send_error_response(int error_code);
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
		HttpRequestHandler(int client_socket, const ServerConfig& config, const Logger* log, ClientData& client_data);
		//	Temporal Method to debug responses for each method
		void send_detailed_response(std::string requested_path);
};


#endif
