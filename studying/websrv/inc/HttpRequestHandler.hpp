#ifndef HTTPREQUESTHANDLER_HPP
#define HTTPREQUESTHANDLER_HPP

#include "webserver.hpp"
#include <string>

typedef enum s_is_file {
	DIR_IS = 0,
	FILE_IS = 1,
	NONE_IS = 3
} t_is_file;

class HttpRequestHandler {
public:
	/**
	 * @brief Handles an HTTP request from a client.
	 *
	 * @param client_socket The file descriptor of the client.
	 * @param config The server configuration for this request.
	 */
	void handle_request(int client_socket, const ServerConfig& config);
	//	Temporal Method to debug responses for each method
	void send_detailed_response(std::string method, const ServerConfig& config, std::string requested_path, int client_socket);
private:
	std::string normalize_request_path(std::string& requested_path, const ServerConfig& config);
	std::string read_http_request(int client_socket);
	std::pair<std::string, std::string> parse_request(const std::string& request);
	void send_error_response(int client_fd, const ServerConfig& config, int error_code);
	static std::string response_header(int code, std::string result, size_t content_size, std::string mime);
	// Handle different methods
	void handle_get(int client_socket, const ServerConfig& config, const std::string& requested_path);
	void handle_post(int client_socket, const ServerConfig& config, const std::string& requested_path);
	void handle_delete(int client_socket, const ServerConfig& config, const std::string& requested_path);
	// Additional helper methods to handle file serving and MIME types (optional to include here)
	std::map<std::string, std::string> create_mime_types();
	std::string get_mime_type(const std::string& path);
};


#endif
