/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/14 13:49:40 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequestHandler.hpp"
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>

/**
 * @brief Get location info using request path
 *
 * Iterates over server location, and evaluate which location config
 * should apply to current request. Load _access and _location_key attributes
 * to make it easier further access.
 *
 * @param path path parsed from request
 */
void HttpRequestHandler::get_location_from_path(const std::string& path)
{
//	TODO: WIP, this method should work with a non heap pointer...
	std::map<std::string, LocationConfig> locations = _config.locations;
	std::string saved_key;
	LocationConfig* result = NULL;

	for (std::map<std::string, LocationConfig>::iterator it = locations.begin(); it != locations.end(); ++it) {
		const std::string& key = it->first;
		if (starts_with(path, key)) {
			if (key.length() > saved_key.length()) {
				result = &it->second;
				saved_key = key;
				_access = it->second.loc_access;
			}
		}
	}
	_location_key = saved_key;
	_location = result;
}


HttpRequestHandler::HttpRequestHandler(int client_socket, const ServerConfig &config): _client_socket(client_socket), _config(config), _access(ACCESS_BAD_REQUEST), _location(NULL)
{
	std::string request = read_http_request();
	std::pair<std::string, std::string> method_and_path = parse_request(request);
//	const LocationConfig& path_config =
	get_location_from_path(method_and_path.second);
	handle_request(method_and_path.first, method_and_path.second);
}
// Temporal method, to send a fix message without further actions, to debug dir and files checks
void HttpRequestHandler::send_detailed_response(const std::string method, const ServerConfig& config, std::string requested_path, int client_socket)
{
	std::string content = "HELLO USING " + method + " FROM PORT : ";
	content += int_to_string(config.port);
	content += " and getting path " + requested_path + "!";
	content += " with full path " + config.server_root + requested_path;
	content += "  and it was evaluated as " + normalize_request_path(requested_path, config).path;
	content += " location found " + _location_key;

	std::string header = response_header(200, content.length(), "text/plain");
	std::string response = header + content;
	send(client_socket, response.c_str(), response.length(), 0);
}

/**
 * @brief Returns a path to get an error page
 *
 * @param requested_path path parsed from request
 * @param config ServerConfig struct
 * TODO: WIP
 * @return t_path struct, that include status and path.
 */
//s_path find_error(const ServerConfig& config)
//{
//
//}

/**
 * @brief Returns a real path, fetching to the document
 *
 * @param requested_path path parsed from request
 * @param config ServerConfig struct
 * TODO: it may be useful return something different, or create a previous check...
 * @return t_path struct, that include status and path.
 */
s_path HttpRequestHandler::normalize_request_path(std::string& requested_path, const ServerConfig& config)
{
	std::string eval_path = config.server_root + requested_path;
	if (is_file(eval_path))
		return (s_path(200, eval_path));
	if (is_dir(eval_path))
	{
		if (eval_path[eval_path.size() - 1] != '/')
			eval_path += "/";
		for (size_t i = 0; i < config.default_pages.size(); i++)
		{
			if (is_file(eval_path + config.default_pages[i]))
				return (s_path(200, eval_path + config.default_pages[i]));
		}
	}
	return (s_path(false, "NONE"));
}

/**
 * @brief Reads the entire HTTP request from the client socket.
 *
 * @param client_socket The file descriptor of the client.
 * @return The full HTTP request as a string.
 */
std::string HttpRequestHandler::read_http_request() {
	char buffer[1024];
	std::string request;
	int valread;

	while ((valread = read(_client_socket, buffer, sizeof(buffer) - 1)) > 0) {
		buffer[valread] = '\0';
		request += buffer;
		if (request.find("\r\n\r\n") != std::string::npos) {
			break;
		}
	}

	return (request);
}

/**
 * @brief Parses the HTTP request to extract the method and requested path.
 *
 * @param request The HTTP request as a string.
 * @return A pair where the first element is the HTTP method (e.g., "GET") and the second is the requested path.
 */
std::pair<std::string, std::string> HttpRequestHandler::parse_request(const std::string& request) {
	std::string method;
	std::string path;

	size_t method_end = request.find(' ');
	if (method_end != std::string::npos) {
		method = request.substr(0, method_end);
		size_t path_end = request.find(' ', method_end + 1);
		if (path_end != std::string::npos) {
			path = request.substr(method_end + 1, path_end - method_end - 1);
		}
	}

	return (std::make_pair(method, path));
}

/**
 * @brief Direct each HTTP method to its own handler
 *
 * @param client_socket Client FD.
 * @param config const reference to ServerConfig struct
 */
void HttpRequestHandler::handle_request(const std::string method, const std::string path) {
	if (_location != NULL)
	{
		std::cout << "parece que si " << _location->loc_root << std::endl;
	}
	if (method == "GET") {
		handle_get(_client_socket, _config, path);
	} else if (method == "POST") {
		handle_post(_client_socket, _config, path);
	} else if (method == "DELETE") {
		handle_delete(_client_socket, _config, path);
	} else {
		send_error_response(_client_socket, _config, 405);  // Method Not Allowed
	}
}

/**
 * @brief GET HTTP method handler
 *
 * @param client_socket Client FD
 * @param config const reference to ServerConfig struct
 * @param requested_path full path from petition
 */
void HttpRequestHandler::handle_get(int client_socket, const ServerConfig& config, const std::string& requested_path) {
	//	TODO: This point, each method returns a message (WIP).
	std::string full_path = config.server_root + requested_path;
	std::ifstream file(full_path.c_str(), std::ios::binary);
	send_detailed_response("GET", config, requested_path, client_socket);
//	std::string content = "HELLO USING GET! from port: ";
//	content += int_to_string(config.port);
//	content += " and getting path " + requested_path;
//	std::string response = response_header(200, "OK", content.length(), get_mime_type(full_path));
//	response += content;
//	send(client_socket, response.c_str(), response.length(), 0);
//	if (file.is_open()) {
//		std::stringstream file_content;
//		file_content << file.rdbuf();
//		std::string content = file_content.str();
//
//		std::string response = response_header(200, "OK", content.length(), get_mime_type(full_path));
//		response += content;
//
//		send(client_socket, response.c_str(), response.length(), 0);
//		file.close();
//	} else {
//		send_error_response(client_socket, config, 404);  // File Not Found
//	}
}

/**
 * @brief POST HTTP method handler
 *
 * @param client_socket Client FD
 * @param config const reference to ServerConfig struct
 * @param requested_path full path from petition
 */
void HttpRequestHandler::handle_post(int client_socket, const ServerConfig& config, const std::string& requested_path) {
	// Read the request body (assuming it's after the headers)
	std::string body = read_http_request();  // Could be refined to separate headers and body

	// For simplicity, we'll just store the body in a file
	std::string full_path = config.server_root + requested_path + "_post_data.txt";  // Save the body as a file
	std::ofstream file(full_path.c_str());

	if (file.is_open()) {
		file << body;
		file.close();

		// Respond to the client with a success message
		std::string response = response_header(200, body.length(), "text/plain");
		response += "POST data received and saved.\n";
		send(client_socket, response.c_str(), response.length(), 0);
	} else {
		send_error_response(client_socket, config, 500);  // Internal Server Error if unable to save file
	}
}

/**
 * @brief DELETE HTTP method handler
 *
 * @param client_socket Client FD
 * @param config const reference to ServerConfig struct
 * @param requested_path full path from petition
 */
void HttpRequestHandler::handle_delete(int client_socket, const ServerConfig& config, const std::string& requested_path) {
	std::string full_path = config.server_root + requested_path;

	// Try to delete the file
	if (remove(full_path.c_str()) == 0) {
		std::string response = response_header(200, 0, "text/plain");
		response += "File deleted successfully.\n";
		send(client_socket, response.c_str(), response.length(), 0);
	} else {
		send_error_response(client_socket, config, 404);  // File Not Found
	}
}

/**
 * @brief Creates a default error body response.
 *
 * @param error_code error code to include at response
 * @return a basic html with error code and detail
 */
std::string HttpRequestHandler::default_plain_error(int error_code)
{
	std::string content = "<!DOCTYPE html>\n";
	content += "<html>\n<head>\n";
	content += "<title>Webserver - Error</title>\n";
	content += "</head>\n<body>\n";
	content += "<h1>Something went wrong...</h1>\n";
	content += "<h2>" + int_to_string(error_code) + " - " + html_codes(error_code) + "</h2>\n";
	content += "</body>\n</html>\n";
	return (content);
}

/**
 * @brief Sends an error response to the client.
 *
 * @param client_fd The file descriptor of the client.
 * @param config The server configuration.
 * @param error_code The HTTP error code to send (e.g., 404).
 */
void HttpRequestHandler::send_error_response(int client_fd, const ServerConfig& config, int error_code) {
	std::string content;
	std::string response;
	std::string type = "text/html";
	std::map<int, std::string>::const_iterator it = config.error_pages.find(error_code);
	if (config.error_pages.empty()) {
	//	Here I just have to handle the logic of "server without err pages -> using webserver defaults
	//	This point I just will send a plain text.
		content = default_plain_error(error_code);
	}
	else if (it != config.error_pages.end())
	{
		std::string error_path = config.server_root + it->second;
		std::ifstream file(error_path.c_str(), std::ios::binary);

		if (file.is_open()) {
			std::stringstream file_content;
			file_content << file.rdbuf();
			file.close();
			content = file_content.str();
			type = get_mime_type(error_path);
		} else {
			content = default_plain_error(error_code);
		}
	}
	response = response_header(error_code, content.length(), type);
	response += content;
	send(client_fd, response.c_str(), response.length(), 0);
}

/**
 * @brief Generates the HTTP response header.
 *
 * @param code The HTTP status code (e.g., 200, 404).
 * @param result The corresponding message (e.g., "OK", "Not Found").
 * @param content_size The size of the content being sent.
 * @param mime The MIME type of the content.
 * @return The HTTP response header as a string.
 */
std::string HttpRequestHandler::response_header(int code, size_t content_size, std::string mime) {
	std::string header = "HTTP/1.1 " + int_to_string(code) + " " + html_codes(code) + "\r\n";
	header += "Content-Length: " + int_to_string(content_size) + "\r\n";
	header += "Content-Type: " + mime + "\r\n";
	header += "\r\n";
	return (header);
}

/**
 * @brief Creates a map to use as static var with extensions and type
 *
 * @return A map that bind an extension with a mime type
 */
std::map<std::string, std::string> HttpRequestHandler::create_mime_types() {
	std::map<std::string, std::string> mime_types;
	mime_types[".html"] = "text/html";
	mime_types[".css"] = "text/css";
	mime_types[".js"] = "application/javascript";
	mime_types[".jpg"] = "image/jpeg";
	mime_types[".jpeg"] = "image/jpeg";
	mime_types[".png"] = "image/png";
	mime_types[".gif"] = "image/gif";
	mime_types[".json"] = "application/json";
	mime_types[".txt"] = "text/plain";
	return (mime_types);
}

/**
 * @brief Given a path, it returns a mime type
 *
 * @param path Full path to a file
 * @return A map that bind an extension with a mime type
 */
std::string HttpRequestHandler::get_mime_type(const std::string& path) {
	static const std::map<std::string, std::string> mime_types = create_mime_types();

	size_t dot_pos = path.find_last_of('.');
	if (dot_pos != std::string::npos) {
		std::string extension = path.substr(dot_pos);
		if (mime_types.find(extension) != mime_types.end()) {
			return (mime_types.at(extension));
		}
	}
	return ("text/plain");
}
