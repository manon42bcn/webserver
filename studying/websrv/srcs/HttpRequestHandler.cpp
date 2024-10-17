/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/17 15:46:11 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequestHandler.hpp"
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>


// Temporal method, to send a fix message without further actions, to debug dir and files checks
void HttpRequestHandler::send_detailed_response(std::string requested_path)
{
	std::string content = "HELLO USING " + method_enum_to_string(_method)+ " FROM PORT : ";
	content += int_to_string(_config.port);
	content += " and getting path " + requested_path + "!";
	content += " with full path " + _config.server_root + requested_path;
	content += "  and it was evaluated as " + normalize_request_path(requested_path).path;
	content += " location found " + _location->loc_root;

	std::string header = response_header(200, content.length(), "text/plain");
	std::string response = header + content;
	send(_client_socket, response.c_str(), response.length(), 0);
}
// -----------------------------------------------------------------
/**
 * @brief HttpRequestHandler constructor
 *
 * Instance and start the lifetime of a HTTP request.
 * For safety issues, all attributes are initiated with wrong request
 * related data.
 *
 * @param client_socket fd client socket.
 * @param config ServerConfig struct.
 */
HttpRequestHandler::HttpRequestHandler(int client_socket, const ServerConfig &config):
	_client_socket(client_socket),
	_config(config),
	_location(NULL),
	_state(ST_INIT),
	_access(ACCESS_FORBIDDEN),
	_http_status(HTTP_CONTINUE),
	_method(METHOD_TO_PARSE)
{
	std::string request = read_http_request();
	std::string path = parse_request_and_method(request);
	// Validate path using locations map and load its configuration as attribute.
	get_location_config(path);
	// Start request process.
	handle_request(path);
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
	_state = ST_READING;
	return (request);
}

/**
 * @brief Parses the HTTP request to extract the method and requested path.
 *
 * This method will parse request, load method to _method, and return path.
 * On error will save Bad Request as http_status.
 *
 * @param request The HTTP request as a string.
 * @return requested path.
 */
std::string HttpRequestHandler::parse_request_and_method(const std::string& request) {
	std::string method;
	std::string path;

	size_t method_end = request.find(' ');
	if (method_end != std::string::npos) {
		method = request.substr(0, method_end);
		_method = method_string_to_enum(method);
		size_t path_end = request.find(' ', method_end + 1);
		if (path_end != std::string::npos) {
			path = request.substr(method_end + 1, path_end - method_end - 1);
		}
	} else {
		_method = METHOD_ERR;
		_http_status = HTTP_BAD_REQUEST;
		_state = -ST_PARSING;
	}
	return (path);
}

/**
 * @brief Get location info using request path
 *
 * Iterates over server location, and evaluate which location config
 * should apply to current request. Load _access state and the pointer
 * to the LocationConfig structure that should be apply to the request.
 *
 * @param path path parsed from request
 */
void HttpRequestHandler::get_location_config(const std::string& path)
{
	std::string saved_key;
	const LocationConfig* result = NULL;

	if (_state < ST_INIT)
		return;
	for (std::map<std::string, LocationConfig>::const_iterator it = _config.locations.begin(); it != _config.locations.end(); ++it) {
		const std::string& key = it->first;
		if (starts_with(path, key)) {
			if (key.length() > saved_key.length()) {
				result = &it->second;
				saved_key = key;
			}
		}
	}
	if (result)
	{
		_location = result;
		_http_status = HTTP_CONTINUE;
		_access = result->loc_access;
		_state = ST_LOAD_LOCATION;
	}
	else
	{
		_location = NULL;
		_state = -ST_LOAD_LOCATION;
	}
}


/**
 * @brief Returns a real path, fetching to the document
 *
 * @param requested_path path parsed from request
 * @param config ServerConfig struct
 * TODO: it may be useful return something different, or create a previous check...
 * @return t_path struct, that include status and path.
 */
s_path HttpRequestHandler::normalize_request_path(const std::string& requested_path) const
{
	//TODO: importante. Estamos asumiendo que location tendrá default pages. Si por config no se especifican, deberá copiar de default..

	std::string eval_path = _location->loc_root + requested_path;

	if (eval_path[eval_path.size() - 1] != '/' && is_file(eval_path))
		return (s_path(HTTP_OK, eval_path));
	if (is_dir(eval_path))
	{
		if (eval_path[eval_path.size() - 1] != '/')
			eval_path += "/";
		for (size_t i = 0; i < _location->loc_default_pages.size(); i++)
		{
			if (is_file(eval_path + _location->loc_default_pages[i]))
				return (s_path(200, eval_path + _location->loc_default_pages[i]));
		}
	}
	return (s_path(HTTP_NOT_FOUND, requested_path));
}


/**
 * @brief Direct each HTTP method to its own handler
 *
 * @param client_socket Client FD.
 * @param config const reference to ServerConfig struct
 */
bool HttpRequestHandler::handle_request(const std::string& path)
{
	if (_state < ST_INIT)
		return (send_error_response(_http_status));
	s_path requested_path = normalize_request_path(path);
	switch ((int)_method) {
		case METHOD_GET:
			handle_get(path);
		case METHOD_POST:
			handle_post(path);
		case METHOD_DELETE:
			handle_delete(path);
		default:
			_http_status = HTTP_METHOD_NOT_ALLOWED;
			send_error_response(HTTP_METHOD_NOT_ALLOWED);
	}
	return (true);
}

/**
 * @brief GET HTTP method handler
 *
 * @param client_socket Client FD
 * @param config const reference to ServerConfig struct
 * @param requested_path full path from petition
 */
void HttpRequestHandler::handle_get(const std::string& requested_path) {
	//	TODO: This point, each method returns a message (WIP).
	std::string full_path = _config.server_root + requested_path;
	std::ifstream file(full_path.c_str(), std::ios::binary);
	send_detailed_response( requested_path);
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
void HttpRequestHandler::handle_post(const std::string& requested_path) {
	// Read the request body (assuming it's after the headers)
	std::string body = read_http_request();  // Could be refined to separate headers and body

	// For simplicity, we'll just store the body in a file
	std::string full_path = _config.server_root + requested_path + "_post_data.txt";  // Save the body as a file
	std::ofstream file(full_path.c_str());

	if (file.is_open()) {
		file << body;
		file.close();

		// Respond to the client with a success message
		std::string response = response_header(200, body.length(), "text/plain");
		response += "POST data received and saved.\n";
		send(_client_socket, response.c_str(), response.length(), 0);
	} else {
		send_error_response(500);  // Internal Server Error if unable to save file
	}
}

/**
 * @brief DELETE HTTP method handler
 *
 * @param client_socket Client FD
 * @param config const reference to ServerConfig struct
 * @param requested_path full path from petition
 */
void HttpRequestHandler::handle_delete(const std::string& requested_path) {
	std::string full_path = _config.server_root + requested_path;

	// Try to delete the file
	if (remove(full_path.c_str()) == 0) {
		std::string response = response_header(200, 0, "text/plain");
		response += "File deleted successfully.\n";
		send(_client_socket, response.c_str(), response.length(), 0);
	} else {
		send_error_response(404);  // File Not Found
	}
}

/**
 * @brief Creates a default error body response.
 *
 * @param error_code error code to include at response
 * @return a basic html with error code and detail
 */
std::string HttpRequestHandler::default_plain_error()
{
	std::string content = "<!DOCTYPE html>\n";
	content += "<html>\n<head>\n";
	content += "<title>Webserver - Error</title>\n";
	content += "</head>\n<body>\n";
	content += "<h1>Something went wrong...</h1>\n";
	content += "<h2>" + int_to_string(_http_status) + " - " + http_status_description(_http_status) + "</h2>\n";
	content += "</body>\n</html>\n";
	return (content);
}


std::string HttpRequestHandler::get_file_content(const std::string& path)
{
	std::string content;
	std::ifstream file(path.c_str(), std::ios::binary);
	_state = -ST_LOAD_FILE;
	if (file.fail())
		_http_status = HTTP_FORBIDDEN;
	if (file.is_open()) {
		std::stringstream file_content;
		file_content << file.rdbuf();
		if (file.bad()){
			_http_status = HTTP_INTERNAL_SERVER_ERROR;
		} else {
			content = file_content.str();
		}
		_state = ST_LOAD_FILE;
		file.close();
	}
	else
		_http_status = HTTP_INTERNAL_SERVER_ERROR;
	return (content);
}

/**
 * @brief Sends an error response to the client.
 *
 * @param client_fd The file descriptor of the client.
 * @param config The server configuration.
 * @param error_code The HTTP error code to send (e.g., 404).
 */
bool HttpRequestHandler::send_error_response(int error_code) {
	std::string content;
	std::string response;
	std::string type = "text/html";
	std::string error_file;

	std::map<int, std::string>::const_iterator it;
	const std::map<int, std::string>* error_pages;

	error_pages = &_config.error_pages;
	it = error_pages->find(error_code);
	if (error_pages->empty() || it == error_pages->end())
		error_file = _config.server_root + it->second;
	if (_location != NULL)
	{
		error_pages = &_location->loc_error_pages;
		it = error_pages->find(error_code);
		if (error_pages->empty() || it == error_pages->end())
			error_file = _location->loc_root + it->second;
	}
	if (!error_file.empty()) {
		std::ifstream file(error_file.c_str(), std::ios::binary);
		content = get_file_content(error_file);

		type = get_mime_type(error_file);
		if (file.is_open()) {
			std::stringstream file_content;
			file_content << file.rdbuf();
			file.close();
			content = file_content.str();
			type = get_mime_type(error_file);
		} else {
			content = default_plain_error();
		}
	} else {
		content = default_plain_error();
	}

	response = response_header(error_code, content.length(), type);
	response += content;
	if (send(_client_socket, response.c_str(), response.length(), 0) == -1)
		return (false);
	return (true);
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
	std::string header = "HTTP/1.1 " + int_to_string(code) + " " + http_status_description((e_http_sts)code) + "\r\n";
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
