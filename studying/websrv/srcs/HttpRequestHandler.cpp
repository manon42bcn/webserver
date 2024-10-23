/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/23 22:37:12 by mporras-         ###   ########.fr       */
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
 * @brief Constructs an HttpRequestHandler to process and validate an HTTP request.
 *
 * This constructor initializes the request handler with the logger and client data,
 * then proceeds to process and validate the HTTP request by calling a series of validation steps.
 * If any validation step fails, it stops the process and logs the failure.
 *
 * @details
 * - The constructor first validates the logger pointer, throwing an exception if it's null.
 * - It reads the HTTP request data, parses it, and then executes a series of validation steps:
 *   1. `parse_method_and_path()`
 *   2. `validate_request()`
 *   3. `get_location_config()`
 *   4. `normalize_request_path()`
 * - If any step fails (i.e., `_sanity` is false), the process is halted and the request is handled accordingly.
 *
 * @param log A pointer to the logger instance used to log actions and errors.
 * @param client_data The data associated with the client, including the server configuration and file descriptor.
 * @throws Logger::NoLoggerPointer if the logger pointer is null.
 */
HttpRequestHandler::HttpRequestHandler(const Logger* log, ClientData& client_data):
	_config(client_data.get_server()->get_config()),
	_log(log),
	_client_data(client_data),
	_location(NULL),
	_fd(_client_data.get_fd().fd),
	_max_request(MAX_REQUEST),
	_sanity(true) {
	if (_log == NULL) {
		throw Logger::NoLoggerPointer();
	}
	size_t i = 0;
	validate_step steps[] = {&HttpRequestHandler::parse_method_and_path,
	                         &HttpRequestHandler::validate_request,
	                         &HttpRequestHandler::get_location_config,
	                         &HttpRequestHandler::normalize_request_path};
	std::string request_data = read_http_request();
	parse_request(request_data);
	_log->log(LOG_DEBUG, RH_NAME,
	          "Parse and Validation Request Process. Start");
	while (i < 4)
	{
		(this->*steps[i])();
		if (!_sanity)
			break;
		i++;
	}
	_log->log(LOG_DEBUG, RH_NAME,
	          "Request Validation Process. End.");
	handle_request();
}

/**
 * @brief Parses an HTTP request into its header and body.
 *
 * This method splits the provided HTTP request string into a header and body, using the
 * delimiter `\r\n\r\n` to identify where the header ends and the body begins.
 *
 * @details
 * - The method first searches for the delimiter `\r\n\r\n` to identify the end of the header.
 * - If the delimiter is found, the header is extracted and stored in `_header`.
 * - If there is additional data after the delimiter, it is stored in `_body`.
 * - If the delimiter is not found, the method disables the sanity check and sets the HTTP status to `HTTP_BAD_REQUEST`.
 *
 * @param request_data The full HTTP request as a string.
 * @return None
 */
void HttpRequestHandler::parse_request(const std::string &request_data) {
	size_t header_end = request_data.find("\r\n\r\n");

	if (header_end != std::string::npos) {
		_header = request_data.substr(0, header_end);
		header_end += 4;
		if (header_end < request_data.length()) {
			_body = request_data.substr(header_end);
		}
	} else {
		turn_off_sanity(HTTP_BAD_REQUEST,
		                "Request parsing error: No header-body delimiter found.");
	}
}

/**
 * @brief Extracts the value of a specific HTTP header field.
 *
 * This method searches the provided header string for a specific key and returns the associated
 * value. The search is case-insensitive, and it assumes the format `key: value`.
 *
 * @details
 * - The method first converts the key and the header string to lowercase for a case-insensitive search.
 * - It searches for the key followed by `": "` to locate the start of the value.
 * - The value is extracted by searching for the next occurrence of `\r\n`, which signifies the end of the value.
 * - If the key is not found, the method returns an empty string.
 *
 * @param header The full HTTP header string.
 * @param key The key for which the value is to be retrieved (e.g., "content-type").
 * @return std::string The value associated with the key, or an empty string if the key is not found.
 */
std::string HttpRequestHandler::get_header_value(std::string header, std::string key) {
	_log->log(LOG_DEBUG, RH_NAME, "Try to get from header " + key);
	header = to_lowercase(header);
	size_t key_pos = header.find(key);
	if (key_pos != std::string::npos) {
		_log->log(LOG_DEBUG, RH_NAME, key + "Found at headers.");
		key_pos += key.length() + 2;
		size_t end_key = header.find("\n\n", key_pos);
		return (header.substr(key_pos, end_key - key_pos));
	}
	_log->log(LOG_DEBUG, RH_NAME, key + "not founded at headers.");
	return ("");
}

/**
 * @brief Reads an HTTP request from the client socket.
 *
 * This method reads data from the client socket and accumulates it into a buffer, building
 * the complete HTTP request. It handles errors such as oversized requests and client disconnections.
 *
 * @details
 * - The method reads data in chunks from the socket, appending it to the `request` string.
 * - If the total request size exceeds `_max_request`, the method sets the HTTP status to
 *   `HTTP_CONTENT_TOO_LARGE` and logs a warning.
 * - If an error occurs during reading, it logs the error, sets the HTTP status to
 *   `HTTP_INTERNAL_SERVER_ERROR`, and returns an empty string.
 * - If the client closes the connection without sending data, the method sets the HTTP status
 *   to `HTTP_CLIENT_CLOSE_REQUEST` and logs the event.
 *
 * @return std::string The full HTTP request as a string, or an empty string if an error occurs.
 */
std::string HttpRequestHandler::read_http_request() {
	char buffer[BUFFER_REQUEST];
	std::string request;
	int         read_byte;
	size_t      size = 0;

	_log->log(LOG_DEBUG, RH_NAME, "Reading http request");
	while ((read_byte = read(_fd, buffer, sizeof(buffer) - 1)) > 0) {
		size += read_byte;
		if (size > _max_request) {
			turn_off_sanity(HTTP_CONTENT_TOO_LARGE, "Request too large.");
			return ("");
		}
		buffer[read_byte] = '\0';
		request += buffer;
		if (read_byte < (int)(sizeof(buffer) - 1))
			break;
	}
	if (read_byte < 0) {
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
		                "Error Reading From Socket." + int_to_string(read_byte));
		return ("");
	}
	if (size == 0) {
		turn_off_sanity(HTTP_CLIENT_CLOSE_REQUEST,
		                "Client Close Request");
		return ("");
	}
	_log->log(LOG_DEBUG, RH_NAME, "Request read.");
	return (request);
}

/**
 * @brief Parses the HTTP request header to extract the method and path.
 *
 * This method analyzes the `_header` string to extract the HTTP method and the requested path.
 * If the header is malformed or if the method or path are invalid, it disables sanity and sets the
 * appropriate HTTP error status.
 *
 * @details
 * - If the header is empty, it immediately disables sanity and sets the status to `HTTP_BAD_REQUEST`.
 * - The method is extracted by finding the first space in the header. If the method is invalid or empty,
 *   sanity is turned off and an error status is set.
 * - The path is extracted by finding the next space in the header. If the path exceeds `URI_MAX` or is missing,
 *   the method sets the status to `HTTP_URI_TOO_LONG` or `HTTP_BAD_REQUEST`, respectively.
 *
 * @return None
 */
void HttpRequestHandler::parse_method_and_path() {
	std::string method;
	std::string path;
	if (_header.empty()) {
		turn_off_sanity(HTTP_BAD_REQUEST, "Header is empty or malformed.");
		return;
	}

	_log->log(LOG_DEBUG, RH_NAME, "Parsing Request to get path and method.");
	size_t method_end = _header.find(' ');
	if (method_end != std::string::npos) {
		method = _header.substr(0, method_end);
		if (method.empty() || (_method = method_string_to_enum(method)) == METHOD_ERR ) {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Error parsing request: Method is empty or not valid.");
			return ;
		}

		size_t path_end = _header.find(' ', method_end + 1);
		if (path_end != std::string::npos) {
			path = _header.substr(method_end + 1, path_end - method_end - 1);
			if (path.size() > URI_MAX) {
				turn_off_sanity(HTTP_URI_TOO_LONG,
				                "Request path too long.");
				return ;
			}
			_log->log(LOG_DEBUG, RH_NAME,
			          "Request header fully parsed.");
			_path = path;
			return ;
		} else {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Error parsing request: missing path.");
			return ;
		}
	} else {
		turn_off_sanity(HTTP_BAD_REQUEST,
		                "Error parsing request: method malformed.");
		_method = METHOD_ERR;
		return ;
	}
}

/**
 * @brief Validates the HTTP request by checking the body and content length.
 *
 * This method checks the consistency of the HTTP request body with the HTTP method.
 * It ensures that the request adheres to the following rules:
 * - Methods such as GET, HEAD, and OPTIONS should not have a body.
 * - Methods such as POST, PUT, and PATCH require a body.
 * - If a body is present, the `Content-Length` header must be valid and must match the actual body size.
 *
 * @details
 * - If the body is present but the method does not allow it (e.g., GET or HEAD), the method disables sanity and sets the status to `HTTP_BAD_REQUEST`.
 * - If the body is missing but the method requires it (e.g., POST), sanity is disabled and the request is rejected.
 * - If the body is present, it verifies that the `Content-Length` header matches the size of the body.
 * - If any validation fails, the method disables sanity and logs the appropriate error message.
 *
 * @return None
 */
void HttpRequestHandler::validate_request() {
	if (!_body.empty()) {
		if (_method == METHOD_GET || _method == METHOD_HEAD || _method == METHOD_OPTIONS) {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Body received with GET, HEAD or OPTION method.");
		}
		std::string content_length = get_header_value(_header, "content-length");
		if (content_length.empty()) {
			turn_off_sanity(HTTP_LENGTH_REQUIRED,
			                "Content Lenght required when body is received.");
		}
		if (is_valid_size_t(content_length)) {
			if (str_to_size_t(content_length) != _body.length()) {
				turn_off_sanity(HTTP_BAD_REQUEST,
				                "Content size header and body size does not match.");
			}
		} else {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Content size header non valid value.");
		}
		_content_type = get_header_value(_header, "content-type");
	} else {
		if (_method == METHOD_POST || _method == METHOD_PUT || _method == METHOD_PATCH) {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Body empty with POST, PUT or PATCH method.");
		}
	}
}

/**
 * @brief Searches for the location configuration that matches the requested path.
 *
 * This method iterates through the available locations and searches for a configuration
 * that matches the requested path. It selects the most specific location if there are multiple matches.
 *
 * @details
 * - The method logs the start of the search and iterates through the `locations` map in `_config`.
 * - It checks if the requested path starts with each location key, and selects the longest matching key.
 * - If a matching location is found, it sets `_location` and updates the access permissions (`_access`).
 * - If no location is found, it calls `turn_off_sanity()` and sets the HTTP status to `HTTP_BAD_REQUEST`.
 *
 * @return None
 */
void HttpRequestHandler::get_location_config() {
	std::string saved_key;
	const LocationConfig* result = NULL;

	_log->log(LOG_DEBUG, RH_NAME, "Searching related location.");
	for (std::map<std::string, LocationConfig>::const_iterator it = _config.locations.begin();
	     it != _config.locations.end(); ++it) {
		const std::string& key = it->first;
		if (starts_with(_path, key)) {
			if (key.length() > saved_key.length()) {
				result = &it->second;
				saved_key = key;
			}
		}
	}
	if (result) {
		_log->log(LOG_DEBUG, RH_NAME, "Location Found: " + saved_key);
		_location = result;
		_access = result->loc_access;
	} else {
		turn_off_sanity(HTTP_BAD_REQUEST, "Location Not Found");
	}
}

/**
 * @brief Normalizes the requested path and identifies the correct file or directory to serve.
 *
 * This method combines the server root, location root, and the requested path to determine
 * the final file path. It checks whether the path corresponds to a file or a directory,
 * and if it is a directory, it searches for default pages in that directory.
 *
 * @details
 * - If the path points to a file, the method sets `_normalized_path` to the file path and the status to `HTTP_OK`.
 * - If the path is a directory, it checks for default pages (e.g., `index.html`) and sets the appropriate path.
 * - If neither a file nor a directory with default pages is found, the method calls `turn_off_sanity()` and sets the HTTP status to `HTTP_NOT_FOUND`.
 *
 * @return None
 */
void HttpRequestHandler::normalize_request_path() {
	std::string eval_path = _config.server_root + _location->loc_root + _path;
	_log->log(LOG_DEBUG, RH_NAME,
			  "Normalize path to get proper file to serve.");

	if (_method == METHOD_POST) {
		_log->log(LOG_DEBUG, RH_NAME,
				  "Path build to a POST request");
		_normalized_path = eval_path;
		validate_post_path();
		return ;
	}

	if (eval_path[eval_path.size() - 1] != '/' && is_file(eval_path)) {
		_log->log(LOG_INFO, RH_NAME, "File found.");
		_normalized_path = eval_path;
		_status = HTTP_OK;
		return ;
	}

	if (is_dir(eval_path)) {
		if (eval_path[eval_path.size() - 1] != '/') {
			eval_path += "/";
		}

		for (size_t i = 0; i < _location->loc_default_pages.size(); i++) {
			if (is_file(eval_path + _location->loc_default_pages[i])) {
				_log->log(LOG_INFO, RH_NAME, "Default File found");
				_normalized_path = eval_path + _location->loc_default_pages[i];
				_status = HTTP_OK;
				return ;
			}
		}
	}
	turn_off_sanity(HTTP_NOT_FOUND,
	                "Requested path not found " + _path);
}

void HttpRequestHandler::validate_post_path() {
	if (_content_type.empty()) {
		turn_off_sanity(HTTP_BAD_REQUEST,
						"Content-Type mandatory in POST Requests.");
		return ;
	}
	if (!valid_mime_type(_path)) {
		turn_off_sanity(HTTP_UNSUPPORTED_MEDIA_TYPE,
						"MIME type not supported.");
		return ;
	}
	if (black_list_extension(_path)) {
		turn_off_sanity(HTTP_FORBIDDEN,
						"Extension file is part of black list.");
		return ;
	}
	std::string path_type = get_mime_type(_path);
	if (path_type != _content_type) {
		turn_off_sanity(HTTP_BAD_REQUEST,
						"Content-Type header does not match with path POST request.");
	}
}

/**
 * @brief Handles an HTTP request by creating a request wrapper and delegating the response.
 *
 * This method wraps the HTTP request details into an `s_request` structure and creates a single instance
 * of `HttpResponseHandler`. If the sanity check (`_sanity`) is false, it sends an error response; otherwise,
 * it processes the request normally.
 *
 * @details
 * - The method creates a request wrapper containing the body, method, path, normalized path, access, sanity, and status.
 * - It then creates a single instance of `HttpResponseHandler` and uses it to either send an error response (if the
 *   sanity check fails) or to handle the request.
 *
 * @return None
 */
void HttpRequestHandler::handle_request() {
	s_request request_wrapper = s_request(_body, _method, _path,
	                                      _normalized_path, _access,
	                                      _sanity, _status);
	HttpResponseHandler response(_location, _log, request_wrapper, _fd);
	if (!_sanity) {
		response.send_error_response();
	} else {
		response.handle_request();
	}
}

/**
 * @brief Disables the request sanity check and sets an HTTP status.
 *
 * This method logs an error detail, turns off the request processing sanity check by setting
 * `_sanity` to false, and updates the HTTP status of the request.
 *
 * @details
 * - The method logs the provided detail as an error.
 * - It sets the `_sanity` flag to false, indicating that the request should no longer be considered "sane."
 * - The HTTP status is updated to the provided value.
 *
 * @param status The HTTP status to be set (e.g., `HTTP_BAD_REQUEST`).
 * @param detail A string detailing the reason for turning off sanity.
 * @return None
 */
void HttpRequestHandler::turn_off_sanity(e_http_sts status, std::string detail) {
	_log->log(LOG_ERROR, RH_NAME, detail);
	_sanity = false;
	_status = status;
}

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
