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
//	std::string full_path = normalize_request_path(requested_path);
	std::string content = "HELLO USING " + method_enum_to_string(_method)+ " FROM PORT : ";
	content += int_to_string(_config.port);
	content += " and getting path " + requested_path + "!";
	content += " with full path " + _config.server_root + requested_path;
	content += "  and it was evaluated as " + normalize_request_path(requested_path).path;
	content += " location found " + _location->loc_root;

	std::string header = response_header(200, content.length(), "text/plain");
	std::string response = header + content;
	send(_fd, response.c_str(), response.length(), 0);
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
 * @param config The configuration of the server (`ServerConfig`) used to process the request.
 * @param log A pointer to the logger instance used to record request processing activity.
 * @param client_data Reference to the client instance to track the process properly.
 *
 * @exception none No exception will be thrown, but exit process if the provided logger pointer is null.
 */
HttpRequestHandler::HttpRequestHandler(const Logger* log, ClientData& client_data):
	_config(client_data.get_server()->get_config()),
	_log(log),
	_client_data(client_data),
	_location(NULL),
	_module("HttpRequestHandler"),
	_state(false),
	_access(ACCESS_FORBIDDEN),
	_http_status(HTTP_I_AM_A_TEAPOT),
	_method(METHOD_TO_PARSE),
	_fd(_client_data.get_fd().fd)
{
	if (_log == NULL) {
		throw Logger::NoLoggerPointer();
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
 * This method reads data from the client socket, accumulating it in a string until
 * either the entire request is received or an error occurs. The method handles errors
 * such as a request that exceeds the maximum allowed size or a client closing the connection.
 *
 * @details
 * - The method reads data in chunks using a buffer and appends it to the `request` string.
 * - If the request size exceeds `MAX_REQUEST`, the method logs a warning and returns an empty string.
 * - If an error occurs during reading, it logs the error and returns an empty string.
 * - The request is considered complete when the method detects the `\r\n\r\n` sequence,
 *   which marks the end of the HTTP headers.
 *
 * @return std::string The full HTTP request as a string, or an empty string if an error occurs.
 */
std::string HttpRequestHandler::read_http_request() {
	char buffer[BUFFER_REQUEST];
	std::string request;
	int         read_byte;
	size_t      size = 0;

	_log->log(LOG_DEBUG, _module, "Reading http request");
	while ((read_byte = (int)read(_fd, buffer, sizeof(buffer) - 1)) > 0) {
		size += read_byte;
		if (size > MAX_REQUEST) {
			_http_status = HTTP_CONTENT_TOO_LARGE;
			_log->log(LOG_WARNING, _module, "Request too large.");
			return ("");
		}
		buffer[read_byte] = '\0';
		request += buffer;
		if (request.find("\r\n\r\n") != std::string::npos) {
			break;
		}
	}
	if (read_byte < 0) {
		_log->log(LOG_ERROR, _module, "Error Reading From Socket.");
		_http_status = HTTP_INTERNAL_SERVER_ERROR;
		return ("");
	}
	if (size == 0) {
		_http_status = HTTP_CLIENT_CLOSE_REQUEST;
		_log->log(LOG_ERROR, _module, "Client Close Request");
		return ("");
	}
	_log->log(LOG_DEBUG, _module, "Request read.");
	_http_status = HTTP_ACCEPTED;
	return (request);
}

/**
 * @brief Parses the HTTP request to extract the method and requested path.
 *
 * This method parses the HTTP request string to identify the HTTP method (e.g., GET, POST)
 * and the requested path. It converts the method to an enumerated value and validates the path.
 * If the method or path are invalid, appropriate error messages are logged and the method
 * returns an empty string.
 *
 * @details
 * - If the request string is empty, an error is logged and the method returns an empty string.
 * - The method extracts the HTTP method from the request and converts it to an enumeration.
 *   If the method is unrecognized, an error log is generated, and the HTTP status is set to
 *   `HTTP_BAD_REQUEST`.
 * - The requested path is validated against a maximum length (`URI_MAX`). If the path exceeds
 *   this length, an error log is generated, and the HTTP status is set to `HTTP_URI_TOO_LONG`.
 * - On success, the requested path is returned and _state will be set to true.
 *
 * @param request The raw HTTP request string received from the client.
 * @return std::string The extracted path, or an empty string if an error occurs.
 */
std::string HttpRequestHandler::parse_request_and_method(const std::string& request) {
	std::string method;
	std::string path;

	if (request.empty())
		return ("");

	_log->log(LOG_DEBUG, _module, "Parsing Request to get path and method.");
	size_t method_end = request.find(' ');
	if (method_end != std::string::npos) {
		method = request.substr(0, method_end);
		if (method.empty() || (_method = method_string_to_enum(method)) == METHOD_ERR ) {
			_log->log(LOG_ERROR, _module,
			          "Error parsing request: Method is empty or not valid.");
			_http_status = HTTP_BAD_REQUEST;
			return ("");
		}

		size_t path_end = request.find(' ', method_end + 1);
		if (path_end != std::string::npos) {
			path = request.substr(method_end + 1, path_end - method_end - 1);

			if (path.size() > URI_MAX) {
				_log->log(LOG_ERROR, _module,
				          "Request path too long.");
				_http_status = HTTP_URI_TOO_LONG;
				return ("");
			}
			_log->log(LOG_DEBUG, _module,
			          "Request parsed.");
		} else {
			_log->log(LOG_ERROR, _module,
			          "Error parsing request: missing path.");
			_http_status = HTTP_BAD_REQUEST;
			return ("");
		}
	} else {
		_log->log(LOG_ERROR, _module,
		          "Error parsing request: method malformed.");
		_method = METHOD_ERR;
		_http_status = HTTP_BAD_REQUEST;
		return ("");
	}
	_state = true;
	return (path);
}

/**
 * @brief Searches for the location configuration based on the requested path.
 *
 * This method iterates through the available locations in the server configuration and
 * finds the best match (i.e., the longest prefix) for the requested path. If a matching
 * location is found, its configuration is loaded and the request state is updated.
 *
 * @details
 * - The method checks the current state to ensure it is valid before proceeding with
 *   the location lookup.
 * - The method searches for the longest matching prefix in the `locations` map.
 * - If a matching location is found, the method sets `_location`, `_http_status`, and `_access`
 *   based on the found location's configuration.
 * - If no location is found, it logs an error, sets `_location` to `NULL`, and updates
 *   the state to reflect the failure.
 *
 * @param path The requested path from the HTTP request.
 * @return None
 */

void HttpRequestHandler::get_location_config(const std::string& path) {
	std::string saved_key;
	const LocationConfig* result = NULL;

	if (!_state) {
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
	} else {
		_log->log(LOG_ERROR, _module, "Location NOT found.");
		_location = NULL;
		_state = false;
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
	std::string eval_path = _config.server_root+ _location->loc_root + requested_path;
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
	if (!_state)
		return (send_error_response());
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
			send_error_response();
	}
	return (true);
}

/**
 * @brief Sends the HTTP response body in fragments to ensure complete transmission.
 *
 * This method constructs the full HTTP response by generating the headers and appending the body.
 * It then sends the response in fragments, ensuring that all bytes are transmitted to the client.
 *
 * @details
 * - The method handles partial sends by keeping track of how many bytes have been sent.
 * - If `send()` returns `-1` it logs the error and returns `false`.
 * - Any exceptions thrown during the sending process are caught, logged, and result in a `false` return value.
 *
 * @param body The content body to be sent in the response.
 * @param path The file path (or resource) that determines the MIME type of the response.
 * @return bool True if the response was sent successfully, false if an error occurred.
 */
bool HttpRequestHandler::sender(const std::string& body, const std::string& path) {
	try {
		std::string response = response_header(_http_status, body.length(), get_mime_type(path));
		response += body;
		int total_sent = 0;
		int to_send = (int)response.length();

		while (total_sent < to_send) {
			int sent_bytes = (int)send(_fd, response.c_str() + total_sent, to_send - total_sent, 0);
			if (sent_bytes == -1) {
				_log->log(LOG_ERROR, _module, "Error sending the response.");
				return (false);
			}
			total_sent += sent_bytes;
		}
		return (true);
	} catch(const std::exception& e) {
		_log->log(LOG_ERROR, _module, e.what());
		return (false);
	}
}

void HttpRequestHandler::handle_get(const std::string& requested_path) {
	s_path file_path = normalize_request_path(requested_path);
	_http_status = file_path.code;

	if (_http_status != HTTP_OK) {
		send_error_response();
		return;
	}
	std::string content = get_file_content(file_path.path);
	if (_http_status == HTTP_OK)
	{
		sender(content, file_path.path);
	} else {
		send_error_response();
	}
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
		send(_fd, response.c_str(), response.length(), 0);
	} else {
		send_error_response();  // Internal Server Error if unable to save file
	}
}


void HttpRequestHandler::handle_delete(const std::string& requested_path) {
	std::string full_path = _config.server_root + requested_path;

	// Try to delete the file
	if (remove(full_path.c_str()) == 0) {
		std::string response = response_header(200, 0, "text/plain");
		response += "File deleted successfully.\n";
		send(_fd, response.c_str(), response.length(), 0);
	} else {
		send_error_response();  // File Not Found
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

	try {
		std::ifstream file(path.c_str(), std::ios::binary);

		if (!file) {
			_log->log(LOG_ERROR, _module, "Failed to open file: " + path);
			_http_status = HTTP_FORBIDDEN;
			return (content);
		}
		file.seekg(0, std::ios::end);
		std::streampos file_size = file.tellg();
		file.seekg(0, std::ios::beg);
		_log->log(LOG_ERROR, _module, "filesize .." + int_to_string((int)file_size));
		content.resize(file_size);

		file.read(&content[0], file_size);

		if (!file) {
			_log->log(LOG_ERROR, _module, "Error reading file: " + path);
			_http_status = HTTP_INTERNAL_SERVER_ERROR;
			return ("");
		}
		file.close();
	} catch (const std::ios_base::failure& e) {
		_log->log(LOG_ERROR, _module, "I/O failure: " + std::string(e.what()));
		_http_status = HTTP_INTERNAL_SERVER_ERROR;
		return ("");
	} catch (const std::exception& e) {
		_log->log(LOG_ERROR, _module, "Exception: " + std::string(e.what()));
		_http_status = HTTP_INTERNAL_SERVER_ERROR;
		return ("");
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
bool HttpRequestHandler::send_error_response() {
	std::string content;
	std::string response;
	std::string type = "text/html";
	std::string error_file;

	content = default_plain_error();
	if (_state)
	{
		std::map<int, std::string>::const_iterator it;
		const std::map<int, std::string>* error_pages = &_location->loc_error_pages;

		it = error_pages->find(_http_status);
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
	response = response_header(_http_status, content.length(), type);
	response += content;
	if (send(_fd, response.c_str(), response.length(), 0) == -1)
	{
		_log->log(LOG_ERROR, _module, "Send error response fails.");
		return (false);
	}
	_log->log(LOG_ERROR, _module, "Error response was sent: " + http_status_description(_http_status));
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
	header << "HTTP/1.1 " << code << " " << http_status_description((e_http_sts)code) << "\r\n"
	       << "Content-Length: " << content_size << "\r\n"
	       << "Content-Type: " <<  mime << "\r\n"
	       << "Connection: close\r\n"  // Cierra la conexión si no es keep-alive
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
	mime_types[".webp"] = "image/webp";
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
