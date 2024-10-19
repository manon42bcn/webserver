/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/17 19:14:28 by mporras-         ###   ########.fr       */
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
 * @brief Constructor for the HttpRequestHandler class.
 *
 * This constructor initializes the `HttpRequestHandler` by reading the client's HTTP request,
 * parsing the method and requested path, and loading the corresponding location configuration
 * for further processing.
 *
 * @details
 * - The constructor reads the HTTP request from the client socket.
 * - The requested path is parsed, and the corresponding location configuration is loaded from
 *   the server configuration.
 * - After the request and path are validated, the request processing begins by calling `handle_request()`.
 *
 * @param client_socket The socket file descriptor for the connected client.
 * @param config The configuration of the server (`ServerConfig`) used to process the request.
 * @param log A pointer to the logger instance used to record request processing activity.
 *
 * @exception none No exception will be thrown, but exit process if the provided logger pointer is null.
 */

HttpRequestHandler::HttpRequestHandler(int client_socket, const ServerConfig &config, Logger* log):
	_client_socket(client_socket),
	_config(config),
	_location(NULL),
	_log(log),
	_module("HttpRequestHandler"),
	_state(ST_INIT),
	_access(ACCESS_FORBIDDEN),
	_http_status(HTTP_CONTINUE),
	_method(METHOD_TO_PARSE)
{
	if (_log == NULL) {
		std::cerr << "Error: Logger cannot be NULL pointer." << std::endl;
		exit(1);
	}
	std::string request = read_http_request();
	std::string path = parse_request_and_method(request);
	// Validate path using locations map and load its configuration as attribute.
	get_location_config(path);
	_log->log(LOG_DEBUG, _module, "Workflow to handle request. Start");
	// Start request process.
	handle_request(path);
}

/**
 * @brief Reads an HTTP request from the client socket.
 *
 * This method reads data from the client socket in chunks and accumulates it in a string
 * until the full HTTP request is received, or an error occurs.
 *
 * @details
 * - The method looks for the end of the HTTP headers (denoted by `\r\n\r\n`) to stop reading.
 * @todo - If the request size exceeds a predefined limit (e.g., 8192 bytes), an error is logged,
 *   and the method returns an empty string.
 * - If an error occurs during the `read()` system call, the method logs the error and returns
 *   an empty string.
 * @todo: - If the client disconnects without sending any data, the method logs an error and returns
 *   an empty string.
 *
 * @return std::string The full HTTP request as a string, or an empty string if an error occurs.
 */

std::string HttpRequestHandler::read_http_request() {
	char buffer[1024];
	std::string request;
	int valread;

	_log->log(LOG_DEBUG, _module, "Reading http request");
	while ((valread = read(_client_socket, buffer, sizeof(buffer) - 1)) > 0) {
		buffer[valread] = '\0';
		request += buffer;
		if (request.find("\r\n\r\n") != std::string::npos) {
			break;
		}
	}
	_log->log(LOG_INFO, _module, "Request read.");
	_state = ST_READING;
	return (request);
}

/**
 * @brief Parses the HTTP request to extract the method and requested path.
 *
 * This method parses the HTTP request string to identify the HTTP method (e.g., GET, POST) and the
 * requested path. It converts the method to an enumerated value and validates the path.
 *
 * @details
 * - The method also checks if the path exceeds a predefined limit (e.g., 2048 characters) to avoid
 *   possible attacks. If the path is too long, the method logs an error and sets the HTTP status
 *   to `HTTP_URI_TOO_LONG`.
 * - In case of any parsing error, an appropriate HTTP status is set, and an empty string is returned.
 *
 * @param request The raw HTTP request string received from the client.
 * @return std::string The extracted path, or an empty string if an error occurs.
 */
std::string HttpRequestHandler::parse_request_and_method(const std::string& request) {
	std::string method;
	std::string path;
	_log->log(LOG_DEBUG, _module, "Parsing Request to get path and method.");
	size_t method_end = request.find(' ');
	if (method_end != std::string::npos) {
		method = request.substr(0, method_end);
		_method = method_string_to_enum(method);

		size_t path_end = request.find(' ', method_end + 1);
		if (path_end != std::string::npos) {
			path = request.substr(method_end + 1, path_end - method_end - 1);

			if (path.size() > 2048) {
				_log->log(LOG_ERROR, _module, "Request path too long.");
				_http_status = HTTP_URI_TOO_LONG;
				return ("");
			}
			_log->log(LOG_DEBUG, _module, "Request parsed.");
		} else {
			_log->log(LOG_ERROR, _module, "Error parsing request: missing path.");
			_http_status = HTTP_BAD_REQUEST;
			_state = ST_PARSING;
			return ("");
		}
	} else {
		_log->log(LOG_ERROR, _module, "Error parsing request: missing method.");
		_method = METHOD_ERR;
		_http_status = HTTP_BAD_REQUEST;
		_state = ST_PARSING;
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
void HttpRequestHandler::get_location_config(const std::string& path) {
	std::string saved_key;
	const LocationConfig* result = NULL;

	if (_state < ST_INIT) {
		_log->log(LOG_WARNING, _module, "Invalid state for location lookup.");
		return;
	}

	_log->log(LOG_DEBUG, _module, "Searching related location.");
	for (std::map<std::string, LocationConfig>::const_iterator it = _config.locations.begin();
	     it != _config.locations.end(); ++it) {
		const std::string& key = it->first;
		if (starts_with(path, key)) {
			if (key.length() > saved_key.length()) {
				result = &it->second;
				saved_key = key;
			}
		}
	}
	if (result) {
		_log->log(LOG_DEBUG, _module, "Location Found: " + saved_key);
		_location = result;
		_http_status = HTTP_CONTINUE;
		_access = result->loc_access;
		_state = ST_LOAD_LOCATION;
	} else {
		_log->log(LOG_ERROR, _module, "Location NOT found.");
		_location = NULL;
		_state = -ST_LOAD_LOCATION;
	}
}

/**
 * @brief Normalizes the requested path and finds the corresponding file or directory.
 *
 * This method normalizes the requested path by combining it with the root directory specified
 * in the location configuration. It checks if the resulting path is a file or a directory and
 * attempts to locate default pages if the path is a directory.
 *
 * @details
 * - If the requested path points to a file, the method returns the normalized path and an HTTP_OK status.
 * - If the path is a directory, the method appends a trailing slash (if missing) and checks for
 *   default pages (e.g., "index.html") in the directory.
 * - If no valid file or directory is found, the method returns an HTTP_NOT_FOUND status.
 *
 * @param requested_path The path requested by the client in the HTTP request.
 * @return s_path A struct containing the HTTP status code and the normalized path.
 */
s_path HttpRequestHandler::normalize_request_path(const std::string& requested_path) const {
	std::string eval_path = _location->loc_root + requested_path;
	_log->log(LOG_DEBUG, _module, "Normalize path to get proper file to serve.");
	if (eval_path[eval_path.size() - 1] != '/' && is_file(eval_path)) {
		_log->log(LOG_INFO, _module, "File found.");
		return (s_path(HTTP_OK, eval_path));
	}

	if (is_dir(eval_path)) {
		if (eval_path[eval_path.size() - 1] != '/') {
			eval_path += "/";
		}

		for (size_t i = 0; i < _location->loc_default_pages.size(); i++) {
			if (is_file(eval_path + _location->loc_default_pages[i])) {
				_log->log(LOG_INFO, _module, "Default File found.");
				return (s_path(HTTP_OK, eval_path + _location->loc_default_pages[i]));
			}
		}
	}

	_log->log(LOG_ERROR, _module, "Requested path not found: " + requested_path);
	return (s_path(HTTP_NOT_FOUND, requested_path));
}

/**
 * @brief Handles the HTTP request based on the method (GET, POST, DELETE).
 *
 * This method processes the HTTP request by first normalizing the requested path and then
 * dispatching the request to the appropriate handler function based on the HTTP method.
 *
 * @details
 * - If the initial state is invalid, the method sends an error response based on the current
 *   HTTP status.
 * - After normalizing the path using `normalize_request_path()`, the method checks if the
 *   normalization succeeded. If the path is invalid or not found, it sends an appropriate
 *   error response (e.g., 404 Not Found).
 * - The request is handled according to the method:
 *   - `GET` requests are passed to `handle_get()`.
 *   - `POST` requests are passed to `handle_post()`.
 *   - `DELETE` requests are passed to `handle_delete()`.
 * - If the HTTP method is not recognized, an `HTTP_METHOD_NOT_ALLOWED` (405) response is sent.
 *
 * @param path The path requested by the client.
 * @return bool True if the request was handled successfully, false if an error occurred.
 */

bool HttpRequestHandler::handle_request(const std::string& path)
{
	if (_state < ST_INIT)
		return (send_error_response(_http_status));
	s_path requested_path = normalize_request_path(path);
	switch ((int)_method) {
		case METHOD_GET:
			_log->log(LOG_DEBUG, _module, "Handle GET request.");
			handle_get(path);
			break;
		case METHOD_POST:
			_log->log(LOG_DEBUG, _module, "Handle POST request.");
			handle_post(path);
			break;
		case METHOD_DELETE:
			_log->log(LOG_DEBUG, _module, "Handle DELETE request.");
			handle_delete(path);
			break;
		default:
			_log->log(LOG_ERROR, _module, "Method not allowed.");
			_http_status = HTTP_METHOD_NOT_ALLOWED;
			send_error_response(HTTP_METHOD_NOT_ALLOWED);
	}
	return (true);
}


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
 * @brief Generates a default HTML error page as a string.
 *
 * This method constructs a basic HTML error page incorporating the HTTP status code
 * and its corresponding description. The page includes a simple message indicating
 * that something went wrong, along with the specific error code and description.
 *
 * @details
 * - The HTML content is built by concatenating strings that represent the HTML structure.
 * - The method uses `_http_status`, which should be set prior to calling this method,
 *   to include the relevant status code in the error page.
 * - The `int_to_string()` function converts the status code to a string, and
 *   `http_status_description()` provides a textual description of the status code.
 *
 * @return std::string The complete HTML error page as a string.
 *
 * @note This method is used to provide a fallback error page when a custom error page
 * is not available or cannot be loaded from the configured error pages.
 */
std::string HttpRequestHandler::default_plain_error() {
	std::ostringstream content;
	content << "<!DOCTYPE html>\n"
	        << "<html>\n<head>\n"
	        << "<title>Webserver - Error</title>\n"
	        << "</head>\n<body>\n"
	        << "<h1>Something went wrong...</h1>\n"
	        << "<h2>" << int_to_string(_http_status) << " - "
	        << http_status_description(_http_status) << "</h2>\n"
	        << "</body>\n</html>\n";
	_log->log(LOG_DEBUG, _module, "Build default error page.");
	return (content.str());
}
// OLD approach...
//std::string HttpRequestHandler::default_plain_error()
//{
//	std::string content = "<!DOCTYPE html>\n";
//	content += "<html>\n<head>\n";
//	content += "<title>Webserver - Error</title>\n";
//	content += "</head>\n<body>\n";
//	content += "<h1>Something went wrong...</h1>\n";
//	content += "<h2>" + int_to_string(_http_status) + " - " + http_status_description(_http_status) + "</h2>\n";
//	content += "</body>\n</html>\n";
//	return (content);
//}

/**
 * @brief Reads the content of a file from the given path.
 *
 * This method attempts to open and read the specified file in binary mode. It handles
 * errors such as file permission issues or failures during the read operation. If any
 * error occurs, an appropriate HTTP status code is set (e.g., `HTTP_FORBIDDEN` for access
 * issues or `HTTP_INTERNAL_SERVER_ERROR` for I/O errors).
 *
 * @details
 * - The file is read using a `std::ifstream` in binary mode to ensure that all types of files
 *   can be processed, including binary files.
 * - The method checks if the file was successfully opened. If not, the method returns an empty
 *   string and sets the HTTP status to `HTTP_FORBIDDEN`.
 * - If the file is opened successfully, it reads the content into a `std::stringstream` and
 *   returns the content as a string. In case of read errors, the HTTP status is set to
 *   `HTTP_INTERNAL_SERVER_ERROR`.
 * - The file is explicitly closed after reading (even though `ifstream` closes it automatically).
 *
 * @param path The file system path to the file.
 * @return std::string The content of the file, or an empty string if an error occurs.
 */

std::string HttpRequestHandler::get_file_content(const std::string& path) {
	std::string content;
	_state = -ST_LOAD_FILE;

	try {
		std::ifstream file(path.c_str(), std::ios::binary);

		if (!file.is_open() || file.fail()) {
			_log->log(LOG_ERROR, _module, "Failed to open file: " + path);
			_http_status = HTTP_FORBIDDEN;
			return (content);
		}

		std::stringstream file_content;
		file_content << file.rdbuf();

		if (file.bad()) {
			_log->log(LOG_ERROR, _module, "Error reading file: " + path);
			_http_status = HTTP_INTERNAL_SERVER_ERROR;
		} else {
			content = file_content.str();
			_state = ST_LOAD_FILE;
		}
		file.close();
	}
	catch (const std::ios_base::failure& e) {
		_log->log(LOG_ERROR, _module, "I/O failure: " + std::string(e.what()));
		_http_status = HTTP_INTERNAL_SERVER_ERROR;
	}
	catch (const std::exception& e) {
		_log->log(LOG_ERROR, _module, "Exception: " + std::string(e.what()));
		_http_status = HTTP_INTERNAL_SERVER_ERROR;
	}

	return (content);
}

/**
 * @brief Sends an HTTP error response to the client.
 *
 * This method generates and sends an error response based on the provided error code.
 * If a custom error page is configured for the given error code, it attempts to load
 * and send that page. If no custom page is found, a default plain HTML error page is sent.
 *
 * @details
 * - It looks for a custom error page in `_location->loc_error_pages`.
 * - If a custom error page is found but cannot be loaded, the method falls back to sending
 *   a default error page.
 * - The MIME type of the error page is determined using `get_mime_type()`, defaulting to
 *   `text/html` if the page is not found.
 * - The method then sends the error response using `send()` to the client. If the send fails,
 *   an error is logged.
 *
 * @param error_code The HTTP status code to send in the response.
 * @rdeturn bool True if the response was sent successfully, false if there was an error.
 */
bool HttpRequestHandler::send_error_response(int error_code) {
	std::string content;
	std::string response;
	std::string type = "text/html";
	std::string error_file;

	content = default_plain_error();
	if (_state != -ST_LOAD_LOCATION)
	{
		std::map<int, std::string>::const_iterator it;
		const std::map<int, std::string>* error_pages = &_location->loc_error_pages;

		it = error_pages->find(error_code);
		if (!error_pages->empty() || it != error_pages->end())
		{
			std::string file_content = get_file_content(error_file);
			if (file_content.empty())
				content = default_plain_error();
			else {
				_log->log(LOG_ERROR, _module, "Custom error page found.");
				content = file_content;
				type = get_mime_type(error_file);
			}
		}
	}
	response = response_header(error_code, content.length(), type);
	response += content;
	if (send(_client_socket, response.c_str(), response.length(), 0) == -1)
	{
		_log->log(LOG_ERROR, _module, "Send error response fails.");
		return (false);
	}
	_log->log(LOG_ERROR, _module, "Error response was sent: " + int_to_string(_http_status));
	return (true);
}

/**
 * @brief Generates an HTTP response header.
 *
 * This method constructs a basic HTTP response header based on the provided status code,
 * content size, and MIME type. It follows the HTTP/1.1 specification and includes the
 * status line, `Content-Length`, and `Content-Type` fields.
 *
 * @details
 * - The method returns the header as a string, including the required carriage return
 *   and line feed (`\r\n`) after each line, and an additional `\r\n` at the end to separate
 *   the headers from the content.
 *
 * @param code The HTTP status code (e.g., 200 for OK, 404 for Not Found).
 * @param content_size The size of the content being sent in the response body.
 * @param mime The MIME type of the content (e.g., `text/html`, `application/json`).
 * @return std::string The complete HTTP response header as a string.
 */
std::string HttpRequestHandler::response_header(int code, size_t content_size, std::string mime) {
	std::ostringstream header;
	header << "HTTP/1.1 " << int_to_string(code) << " " << http_status_description((e_http_sts)code) << "\r\n"
	       << "Content-Length: " << int_to_string((int)content_size) << "\r\n"
	       << "Content-Type: " <<  mime << "\r\n"
	       << "\r\n";
	return (header.str());
}

/**
 * @brief Creates and returns a map of file extensions to MIME types.
 *
 * This method generates a map that associates common file extensions (e.g., ".html", ".jpg")
 * with their corresponding MIME types (e.g., "text/html", "image/jpeg"). The map is used
 * to determine the `Content-Type` header when serving files.
 *
 * @details
 * - The method ensures that the map is initialized only once, using a static map to avoid
 *   recreating the map on each call. If additional MIME types are required, they can be
 *   added to the map.
 * - Common MIME types such as `text/html`, `application/javascript`, and `image/jpeg` are included.
 *
 * @return std::map<std::string, std::string> A map that associates file extensions with their MIME types.
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
 * @brief Retrieves the MIME type based on the file extension.
 *
 * This method looks up the MIME type corresponding to the file extension in the provided path.
 * If the file extension is recognized, the associated MIME type is returned. If the extension
 * is not recognized, it defaults to `text/plain`.
 *
 * @details
 * - The method extracts the file extension by searching for the last '.' character in the path.
 * - If the extension is found in the `mime_types` map, the corresponding MIME type is returned.
 * - If no recognized extension is found, the default MIME type `text/plain` is returned.
 *
 * @param path The file system path to the file.
 * @return std::string The MIME type corresponding to the file extension, or `text/plain` if not found.
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
	_log->log(LOG_DEBUG, _module, "File extension not recognized, defaulting to text/plain.");
	return ("text/plain");
}
