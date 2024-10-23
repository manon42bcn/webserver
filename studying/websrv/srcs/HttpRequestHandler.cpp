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
	_log->log(LOG_DEBUG, RH_NAME, "Workflow to handle request. Start");
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

	_log->log(LOG_DEBUG, RH_NAME, "Reading http request");
	while ((read_byte = (int)read(_fd, buffer, sizeof(buffer) - 1)) > 0) {
		size += read_byte;
		if (size > MAX_REQUEST) {
			_http_status = HTTP_CONTENT_TOO_LARGE;
			_log->log(LOG_WARNING, RH_NAME, "Request too large.");
			return ("");
		}
		buffer[read_byte] = '\0';
		request += buffer;
		if (request.find("\r\n\r\n") != std::string::npos) {
			break;
		}
	}
	if (read_byte < 0) {
		_log->log(LOG_ERROR, RH_NAME, "Error Reading From Socket.");
		_http_status = HTTP_INTERNAL_SERVER_ERROR;
		return ("");
	}
	if (size == 0) {
		_http_status = HTTP_CLIENT_CLOSE_REQUEST;
		_log->log(LOG_ERROR, RH_NAME, "Client Close Request");
		return ("");
	}
	_log->log(LOG_DEBUG, RH_NAME, "Request read.");
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

	_log->log(LOG_DEBUG, RH_NAME, "Parsing Request to get path and method.");
	size_t method_end = request.find(' ');
	if (method_end != std::string::npos) {
		method = request.substr(0, method_end);
		if (method.empty() || (_method = method_string_to_enum(method)) == METHOD_ERR ) {
			_log->log(LOG_ERROR, RH_NAME,
			          "Error parsing request: Method is empty or not valid.");
			_http_status = HTTP_BAD_REQUEST;
			return ("");
		}

		size_t path_end = request.find(' ', method_end + 1);
		if (path_end != std::string::npos) {
			path = request.substr(method_end + 1, path_end - method_end - 1);

			if (path.size() > URI_MAX) {
				_log->log(LOG_ERROR, RH_NAME,
				          "Request path too long.");
				_http_status = HTTP_URI_TOO_LONG;
				return ("");
			}
			_log->log(LOG_DEBUG, RH_NAME,
			          "Request parsed.");
		} else {
			_log->log(LOG_ERROR, RH_NAME,
			          "Error parsing request: missing path.");
			_http_status = HTTP_BAD_REQUEST;
			return ("");
		}
	} else {
		_log->log(LOG_ERROR, RH_NAME,
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
		_log->log(LOG_WARNING, RH_NAME, "Invalid state for location lookup.");
		return;
	}

	_log->log(LOG_DEBUG, RH_NAME, "Searching related location.");
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
		_log->log(LOG_DEBUG, RH_NAME, "Location Found: " + saved_key);
		_location = result;
		_http_status = HTTP_CONTINUE;
		_access = result->loc_access;
	} else {
		_log->log(LOG_ERROR, RH_NAME, "Location NOT found.");
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
	_log->log(LOG_DEBUG, RH_NAME, "Normalize path to get proper file to serve.");
	_log->log(LOG_DEBUG, RH_NAME, "Eval path: " + eval_path);
	_log->log(LOG_DEBUG, RH_NAME, "_config.server_root: " + _config.server_root);
	_log->log(LOG_DEBUG, RH_NAME, "_location->loc_root: " + _location->loc_root);

	// FIRST ERROR IN 42 IS here, with _http_status value 404, and path "/"
	// /home/vaguilar/Desktop/webserver/studying/websrvdata
	// EN LUGAR DE
	// /home/vaguilar/Desktop/webserver/studying/websrv/data
	eval_path = "/home/vaguilar/Desktop/webserver/studying/websrv/data";
	if (eval_path[eval_path.size() - 1] != '/' && is_file(eval_path)) {
		_log->log(LOG_INFO, RH_NAME, "File found.");
		return (s_path(HTTP_OK, true, eval_path));
	}

	if (is_dir(eval_path)) {
		if (eval_path[eval_path.size() - 1] != '/') {
			eval_path += "/";
		}

		for (size_t i = 0; i < _location->loc_default_pages.size(); i++) {
			if (is_file(eval_path + _location->loc_default_pages[i])) {
				_log->log(LOG_INFO, RH_NAME, "Default File found.");
				return (s_path(HTTP_OK, true, eval_path + _location->loc_default_pages[i]));
			}
		}
	}

	_log->log(LOG_ERROR, RH_NAME, "Requested path not found: " + requested_path);
	return (s_path(HTTP_NOT_FOUND, false, requested_path));
}

/**
 * @brief Handles an HTTP request by delegating the response to `HttpResponseHandler`.
 *
 * This method normalizes the requested path and delegates the handling of the request
 * to an instance of `HttpResponseHandler`. If the internal state is invalid, it sends an error response.
 *
 * @details
 * - The method first normalizes the path using `normalize_request_path()`.
 * - It creates an instance of `HttpResponseHandler` to handle the request based on the method, path, and state.
 * - If the internal `_state` is invalid, it calls `send_error_response()` to generate and send an error response.
 * - Otherwise, it delegates the request handling to `HttpResponseHandler::handle_request()`.
 *
 * @param path The requested path from the client.
 * @return bool True if the request was handled successfully, false otherwise.
 */
bool HttpRequestHandler::handle_request(const std::string& path)
{
	s_path eval_path = normalize_request_path(path);
	_http_status = eval_path.code;
	HttpResponseHandler response(_fd, _http_status, _location, _log, _method, eval_path);
	if (!_state)
		return (response.send_error_response());
	return (response.handle_request());
}
//
//void HttpRequestHandler::handle_post(const std::string& requested_path) {
//	// Read the request body (assuming it's after the headers)
//	std::string body = read_http_request();  // Could be refined to separate headers and body
//
//	// For simplicity, we'll just store the body in a file
//	std::string full_path = _config.server_root + requested_path + "_post_data.txt";  // Save the body as a file
//	std::ofstream file(full_path.c_str());
//
//	if (file.is_open()) {
//		file << body;
//		file.close();
//
//		// Respond to the client with a success message
//		std::string response = response_header(200, body.length(), "text/plain");
//		response += "POST data received and saved.\n";
//		send(_fd, response.c_str(), response.length(), 0);
//	} else {
//		send_error_response();  // Internal Server Error if unable to save file
//	}
//}
//
//
//void HttpRequestHandler::handle_delete(const std::string& requested_path) {
//	std::string full_path = _config.server_root + requested_path;
//
//	// Try to delete the file
//	if (remove(full_path.c_str()) == 0) {
//		std::string response = response_header(200, 0, "text/plain");
//		response += "File deleted successfully.\n";
//		send(_fd, response.c_str(), response.length(), 0);
//	} else {
//		send_error_response();  // File Not Found
//	}
//}

//// Temporal method, to send a fix message without further actions, to debug dir and files checks
//void HttpRequestHandler::send_detailed_response(std::string requested_path)
//{
//	//	std::string full_path = normalize_request_path(requested_path);
//	std::string content = "HELLO USING " + method_enum_to_string(_method)+ " FROM PORT : ";
//	content += int_to_string(_config.port);
//	content += " and getting path " + requested_path + "!";
//	content += " with full path " + _config.server_root + requested_path;
//	content += "  and it was evaluated as " + normalize_request_path(requested_path).path;
//	content += " location found " + _location->loc_root;
//
//	std::string header = response_header(200, content.length(), "text/plain");
//	std::string response = header + content;
//	send(_fd, response.c_str(), response.length(), 0);
//}
//// -----------------------------------------------------------------
