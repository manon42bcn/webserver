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


HttpRequestHandler::HttpRequestHandler(int client_socket, const ServerConfig &config): _client_socket(client_socket), _config(config)
{
	_access = ACCESS_FORBIDDEN;
	_request = read_http_request();
	std::pair<std::string, std::string> method_and_path = parse_request();
	_method = method_and_path.first;
	_path = method_and_path.second;

	handle_request();
}
// Temporal method, to send a fix message without further actions, to debug dir and files checks
void HttpRequestHandler::send_detailed_response(std::string method, const ServerConfig& config, std::string requested_path, int client_socket)
{
	std::string content = "HELLO USING " + method + " FROM PORT : ";
	content += int_to_string(config.port);
	content += " and getting path " + requested_path + "!";
	content += " with full path " + config.server_root + requested_path;
	content += "  and it was evaluated as " + normalize_request_path(requested_path, config).path;

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

	return request;
}

/**
 * @brief Parses the HTTP request to extract the method and requested path.
 *
 * @param request The HTTP request as a string.
 * @return A pair where the first element is the HTTP method (e.g., "GET") and the second is the requested path.
 */
std::pair<std::string, std::string> HttpRequestHandler::parse_request() {
	std::string method;
	std::string path;

	std::cout << _request << std::endl;
	size_t method_end = _request.find(" ");
	if (method_end != std::string::npos) {
		method = _request.substr(0, method_end);
		size_t path_end = _request.find(" ", method_end + 1);
		if (path_end != std::string::npos) {
			path = _request.substr(method_end + 1, path_end - method_end - 1);
		}
	}

	return std::make_pair(method, path);
}

/**
 * @brief Direct each HTTP method to its own handler
 *
 * @param client_socket Client FD.
 * @param config const reference to ServerConfig struct
 */
void HttpRequestHandler::handle_request() {

	if (_method == "GET") {
		handle_get(_client_socket, _config, _path);
	} else if (_method == "POST") {
		handle_post(_client_socket, _config, _path);
	} else if (_method == "DELETE") {
		handle_delete(_client_socket, _config, _path);
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
//
///**
// * @brief Read the request from the socket.
// *
// * @param client_socket Client FD
// * @return std::string Full request string format.
// */
//std::string HttpRequestHandler::read_http_request(int client_socket) {
//	char buffer[1024];
//	std::string request;
//	int valread;
//
//	while ((valread = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
//		buffer[valread] = '\0';
//		request += buffer;
//		if (request.find("\r\n\r\n") != std::string::npos) {
//			break;
//		}
//	}
//
//	return (request);
//}
//
///**
// * @brief Parse requested route
// *
// * @param request Request string format
// * @return std::string Requested route
// */
//std::string HttpRequestHandler::parse_requested_path(const std::string& request) {
//	size_t pos = request.find(" ");
//	if (pos != std::string::npos) {
//		size_t end_pos = request.find(" ", pos + 1);
//		if (end_pos != std::string::npos) {
//			return request.substr(pos + 1, end_pos - pos - 1);
//		}
//	}
//	return "/";
//}
//
///**
// * @brief HTTP Request handler
// *
// * @param client_socket Client FD.
// * @param config Server config struct
// */
//void HttpRequestHandler::handle_request(int client_socket, const ServerConfig config) {
//	std::string request = read_http_request(client_socket);
//	std::string requested_path = parse_requested_path(request);
//
//	// Manejo de la solicitud para el root "/"
//	if (requested_path == "/") {
//		for (size_t i = 0; i < config.default_pages.size(); ++i) {
//			std::string full_path = config.server_root + "/" + config.default_pages[i];
//			std::ifstream file(full_path.c_str(), std::ios::binary);
//			if (file.is_open()) {
//				// Leer y enviar el archivo encontrado
//				std::stringstream file_content;
//				file_content << file.rdbuf();
//				std::string content = file_content.str();
//
//				std::string response = response_header(200, "OK", content.length(), get_mime_type(full_path));
//				response += content;
//				send(client_socket, response.c_str(), response.length(), 0);
//				file.close();
//				return;
//			}
//		}
//	}
//
//	// Manejo de solicitudes a subdirectorios "/casos/"
//	if (requested_path.back() == '/') {
//		for (size_t i = 0; i < config.default_pages.size(); ++i) {
//			std::string full_path = config.server_root + requested_path + config.default_pages[i];
//			std::ifstream file(full_path.c_str(), std::ios::binary);
//			if (file.is_open()) {
//				// Leer y enviar el archivo encontrado
//				std::stringstream file_content;
//				file_content << file.rdbuf();
//				std::string content = file_content.str();
//
//				std::string response = response_header(200, "OK", content.length(), get_mime_type(full_path));
//				response += content;
//				send(client_socket, response.c_str(), response.length(), 0);
//				file.close();
//				return;
//			}
//		}
//	}
//
//	// Manejo de solicitudes a archivos específicos
//	std::string full_path = config.server_root + requested_path;
//	std::ifstream file(full_path.c_str(), std::ios::binary);
//
//	if (file.is_open()) {
//		// Leer y enviar el archivo solicitado
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
//		send_error_response(client_socket, config, 404);
//	}
//}
//
//
///**
// * @brief Send an error page to the client
// *
// * If the error page does not exist, it will send a text to avoid server silent
// *
// * @param client_fd client FD
// * @param config ServerConfig struct
// * @param error_code Error code.
// */
//void HttpRequestHandler::send_error_response(int client_fd, const ServerConfig config, int error_code) {
//	std::string error_page_path;
//	bool file_found = false;
//
//	// Intentar obtener la página de error del archivo
//	if (config.error_pages.find(error_code) != config.error_pages.end()) {
//		error_page_path = config.server_root + config.error_pages.at(error_code);
//		std::ifstream file(error_page_path.c_str(), std::ios::binary);
//		if (file.is_open()) {
//			// Leer y enviar el archivo de error
//			std::stringstream file_content;
//			file_content << file.rdbuf();
//			std::string content = file_content.str();
//
//			std::string response = response_header(error_code, "Error", content.length(), "text/plain");
//			response += content;
//
//			send(client_fd, response.c_str(), response.length(), 0);
//			file.close();
//			file_found = true;
//		}
//	}
//
//	// Si no se encontró el archivo, enviar un texto predeterminado
//	if (!file_found) {
//		std::string default_text = "Error " + int_to_string(error_code) + " - Error page not found.";
//		std::string response = response_header(error_code, "Error", default_text.length(), "text/plain");
//		response += default_text;
//
//		send(client_fd, response.c_str(), response.length(), 0);
//	}
//}
//
///**
// * @brief Creates the normal header to include at response
// *
// * @param code HTML status code to include (200, 404, etc)
// * @param result Verbose details to include with status code (OK, Error..)
// * @param content_size len of content to include at response
// */
//std::string HttpRequestHandler::response_header(int code,
//												std::string result,
//												size_t content_size,
//												std::string mime)
//{
//	std::string header = "HTTP/1.1 " + int_to_string(code) + " " + result + "\r\n";
//	header += "Content-Length: " + int_to_string(content_size) + "\r\n";
//	header += "Content-Type: \"" + mime + "\"\r\n";
//	header += "\r\n";
//	return (header);
//}
