/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/14 13:53:14 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTHANDLER_HPP
#define HTTPREQUESTHANDLER_HPP

#include "webserver.hpp"
#include <string>

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
class HttpRequestHandler {
	private:
		int                 _client_socket;
		std::string         _request;
		std::string         _method;
		e_access            _access;
		std::string         _path;
		const ServerConfig& _config;

		s_path normalize_request_path(std::string& requested_path, const ServerConfig& config);
		std::string read_http_request();
		std::pair<std::string, std::string> parse_request();
		std::string default_plain_error(int error_code);
		void send_error_response(int client_fd, const ServerConfig& config, int error_code);
		static std::string response_header(int code, size_t content_size, std::string mime);
		// Handle different methods
		void handle_get(int client_socket, const ServerConfig& config, const std::string& requested_path);
		void handle_post(int client_socket, const ServerConfig& config, const std::string& requested_path);
		void handle_delete(int client_socket, const ServerConfig& config, const std::string& requested_path);
		// Additional helper methods to handle file serving and MIME types (optional to include here)
		std::map<std::string, std::string> create_mime_types();
		std::string get_mime_type(const std::string& path);
	public:
		HttpRequestHandler(int client_socket, const ServerConfig& config);
		void handle_request();
		//	Temporal Method to debug responses for each method
		void send_detailed_response(std::string method, const ServerConfig& config, std::string requested_path, int client_socket);
};


#endif
