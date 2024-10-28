/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/28 13:39:29 by mporras-         ###   ########.fr       */
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
HttpRequestHandler::HttpRequestHandler(const Logger* log, ClientData* client_data):
	_config(client_data->get_server()->get_config()),
	_log(log),
	_client_data(client_data),
	_location(NULL),
	_fd(_client_data->get_fd().fd),
	_max_request(MAX_REQUEST),
	_sanity(true) {
	if (_log == NULL) {
		throw Logger::NoLoggerPointer();
	}
	size_t i = 0;
	validate_step steps[] = {&HttpRequestHandler::read_request_header,
							 &HttpRequestHandler::parse_header,
	                         &HttpRequestHandler::parse_method_and_path,
	                         &HttpRequestHandler::load_header_data,
	                         &HttpRequestHandler::load_content,
	                         &HttpRequestHandler::validate_request,
	                         &HttpRequestHandler::get_location_config,
	                         &HttpRequestHandler::normalize_request_path};

	_log->log(LOG_DEBUG, RH_NAME,
	          "Parse and Validation Request Process. Start");
	while (i < (sizeof(steps) / sizeof(validate_step)))
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

HttpRequestHandler::~HttpRequestHandler() {
	_client_data = NULL;
	_location = NULL;
	_log->log(LOG_DEBUG, RH_NAME,
	          "HttpRequestHandler resources clean up.");
	_log = NULL;
}

/**
 * @brief Reads the HTTP request header from the client socket with a timeout check.
 *
 * This method reads the header data from the client socket in chunks and appends it to `_request`.
 * It stops reading when the header size exceeds `_max_request`, the timeout defined in `_client_data`
 * expires, or when the header-body delimiter `\r\n\r\n` is found.
 *
 * @details
 * - The method reads data in chunks using `recv` and appends it to `_request`.
 * - If the accumulated size exceeds `_max_request`, it disables sanity and sets the HTTP status to `HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE`.
 * - If the timeout in `_client_data` expires, it disables sanity and sets the status to `HTTP_REQUEST_TIMEOUT`.
 * - If the client closes the connection, it sets the status to `HTTP_CLIENT_CLOSE_REQUEST`.
 * - On successful read, it resets the timeout counter in `_client_data`.
 *
 * @return None
 */
void HttpRequestHandler::read_request_header() {
	char buffer[BUFFER_REQUEST];
	int         read_byte;
	size_t      size = 0;

	_log->log(LOG_DEBUG, RH_NAME, "Reading http request");
	while ((read_byte = recv(_fd, buffer, sizeof(buffer), 0)) > 0) {
		size += read_byte;
		if (size > _max_request) {
			turn_off_sanity(HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE,
			                "Request Header too large.");
			return ;
		}
		_request.append(buffer, read_byte);
		if (_request.find("\r\n\r\n") != std::string::npos) {
			break ;
		}
		if (!_client_data->chronos()) {
			turn_off_sanity(HTTP_REQUEST_TIMEOUT,
			                "Request Timeout.");
			return ;
		}
	}
	if (read_byte < 0 && size == 0) {
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
		                "Error Reading From Socket. " + int_to_string(read_byte));
		return ;
	}
	if (size == 0) {
		turn_off_sanity(HTTP_CLIENT_CLOSE_REQUEST,
		                "Client Close Request");
		return ;
	}
	_client_data->chronos_reset();
	_log->log(LOG_DEBUG, RH_NAME,
	          "Request read.");
}

/**
 * @brief Parses the HTTP request to separate the header from the body, ensuring the header is not empty.
 *
 * This method locates the header-body delimiter `\r\n\r\n` within the request and separates the header portion.
 * It verifies that the header is not empty before continuing, assigning any remaining data as the body in `_request`.
 *
 * @details
 * - If the delimiter `\r\n\r\n` is found, the method extracts and assigns the header to `_header`.
 * - If `_header` is empty, the method disables sanity and sets the status to `HTTP_BAD_REQUEST`.
 * - The remaining content after the delimiter is assigned to `_request` as the body.
 * - If the delimiter is not found, it disables sanity and sets the status to `HTTP_BAD_REQUEST`.
 *
 * @return None
 */
void HttpRequestHandler::parse_header() {
	size_t header_end = _request.find("\r\n\r\n");

	if (header_end != std::string::npos) {
		_header = _request.substr(0, header_end);
		if (_header.empty()) {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Request Header is empty.");
			return ;
		}
		_log->log(LOG_DEBUG, RH_NAME,
		          "Header successfully parsed.");
		header_end += 4;
		_request = _request.substr(header_end);
	} else {
		turn_off_sanity(HTTP_BAD_REQUEST,
		                "Request parsing error: No header-body delimiter found.");
	}
}

/**
 * @brief Parses the HTTP request header to extract the method and path.
 *
 * This method analyzes the `_header` string to extract the HTTP method and the requested path.
 * If the header is malformed or if the method or path are invalid, it disables sanity and sets the
 * appropriate HTTP error status.
 *
 * @details
 * - If the header does not contain a space character to separate the method, it sets the status to `HTTP_BAD_REQUEST`.
 * - The method portion is extracted and validated; if it is empty or invalid, it disables sanity.
 * - The path is extracted next; if it exceeds `URI_MAX`, it disables sanity and sets the status to `HTTP_URI_TOO_LONG`.
 * - Upon successful parsing, the method and path are logged for tracking.
 *
 * @return None
 */
void HttpRequestHandler::parse_method_and_path() {
	std::string method;
	std::string path;

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
 * @brief Parses the HTTP request header to extract the method and path.
 *
 * This method analyzes the `_header` string to extract the HTTP method and the requested path.
 * If the header is malformed or if the method or path are invalid, it disables sanity and sets the
 * appropriate HTTP error status.
 *
 * @details
 * - If the header does not contain a space character to separate the method, it sets the status to `HTTP_BAD_REQUEST`.
 * - The method portion is extracted and validated; if it is empty or invalid, it disables sanity.
 * - The path is extracted next; if it exceeds `URI_MAX`, it disables sanity and sets the status to `HTTP_URI_TOO_LONG`.
 * - Upon successful parsing, the method and path are logged for tracking.
 *
 * @return None
 */
void HttpRequestHandler::load_header_data() {
	std::string content_length = get_header_value(_header, "content-length:", "\r\n");
	_content_type = get_header_value(_header, "content-type:", "\r\n");

	if (!content_length.empty()){
		if (is_valid_size_t(content_length)) {
			_content_leght = str_to_size_t(content_length);
		} else {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Content-Length malformed.");
		}
	} else {
		_content_leght = 0;
	}
	if (!_content_type.empty()) {
		if (starts_with(_content_type, "multipart")) {
			_boundary = get_header_value(_content_type, "boundary", "\r\n");
			if (_boundary.empty()) {
				turn_off_sanity(HTTP_BAD_REQUEST,
				                "Boundary malformed or not present with a multipart Content-Type.");
			}
			size_t end_type = _content_type.find(';');
			if (end_type != std::string::npos) {
				_content_type = _content_type.substr(0, end_type);
			}
		}
	}
}

/**
 * @brief Loads the HTTP request content from the client socket.
 *
 * This method reads the body content of the HTTP request up to the specified `Content-Length`.
 * It verifies that the body does not exceed `_max_request`, and checks for timeout during the reading process.
 *
 * @details
 * - If `Content-Length` is zero, the method logs this information and returns immediately.
 * - Reads in chunks from the socket, accumulating the data in `_request` until the specified length or a timeout occurs.
 * - If the accumulated size exceeds `_max_request`, it disables sanity and sets the status to `HTTP_CONTENT_TOO_LARGE`.
 * - If the client closes the connection or an error occurs, it sets the appropriate HTTP status.
 * - On successful read, it assigns the accumulated content to `_body`.
 *
 * @return None
 */
void HttpRequestHandler::load_content() {
	if (_content_leght == 0) {
		_log->log(LOG_INFO, RH_NAME,
		          "No Content-Length to read from FD.");
		return ;
	}

	char buffer[BUFFER_REQUEST];
	int         read_byte;
	size_t      size = 0;
	size_t      to_read = _content_leght - _request.length();

	while (to_read > 0) {
		read_byte = recv(_fd, buffer, sizeof(buffer) - 1, 0);
		if (read_byte < 0) {
			continue ;
		}
		size += read_byte;
		to_read -= read_byte;
		if (size > _max_request) {
			turn_off_sanity(HTTP_CONTENT_TOO_LARGE,
			                "Body Content too Large.");
			return ;
		}
		_request.append(buffer, read_byte);
		if (!_client_data->chronos()) {
			turn_off_sanity(HTTP_REQUEST_TIMEOUT,
			                "Request Timeout.");
			return ;
		}
	}
	if (read_byte < 0 && size == 0) {
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
		                "Error Reading From Socket. " + int_to_string(read_byte));
		return ;
	}
	if (size == 0) {
		turn_off_sanity(HTTP_CLIENT_CLOSE_REQUEST,
		                "Client Close Request");
		return ;
	}
	_client_data->chronos_reset();
	_body = _request;
	_log->log(LOG_DEBUG, RH_NAME, "Request read.");
}

///**
// * @brief Extracts the value of a specific HTTP header field.
// *
// * This method searches the provided header string for a specific key and returns the associated
// * value. The search is case-insensitive, and it assumes the format `key: value`.
// *
// * @details
// * - The method first converts the key and the header string to lowercase for a case-insensitive search.
// * - The value is extracted by searching for the next occurrence of `\r\n`, which signifies the end of the value.
// * - If the key is not found, the method returns an empty string.
// *
// * @param haystack The HTTP Header format string to be searched over it.
// * @param needle The key for which the value is to be retrieved (e.g., "content-type").
// * @return std::string The value associated with the key, or an empty string if the key is not found.
// */
//std::string HttpRequestHandler::get_header_value(std::string& haystack, std::string needle) {
//	_log->log(LOG_DEBUG, RH_NAME, "Try to get from header " + needle);
//	std::string lower_header = to_lowercase(haystack);
//	size_t key_pos = lower_header.find(needle);
//
//	if (key_pos != std::string::npos) {
//		_log->log(LOG_DEBUG, RH_NAME, needle + "Found at headers.");
//		key_pos += needle.length() + 1;
//		size_t end_key = lower_header.find("\r\n", key_pos);
//		if (end_key == std::string::npos) {
//			return (haystack.substr(key_pos));
//		} else {
//			return (haystack.substr(key_pos, end_key - key_pos));
//		}
//	}
//	_log->log(LOG_DEBUG, RH_NAME, needle + "not founded at headers.");
//	return ("");
//}

/**
 * @brief Validates the HTTP request to ensure body content is consistent with the method.
 *
 * This method checks that the presence or absence of a request body matches the HTTP method requirements.
 * - Methods GET, HEAD, and OPTIONS should not have a body.
 * - Methods POST, PUT, and PATCH require a body.
 *
 * @details
 * - If a body is present with GET, HEAD, or OPTIONS, it disables sanity and sets the status to `HTTP_BAD_REQUEST`.
 * - If a body is missing with POST, PUT, or PATCH, it also disables sanity and sets the status to `HTTP_BAD_REQUEST`.
 * - Logs warnings indicating mismatches between the method and body presence.
 *
 * @return None
 */
void HttpRequestHandler::validate_request() {
	if (!_body.empty()) {
		if (_method == METHOD_GET || _method == METHOD_HEAD || _method == METHOD_OPTIONS) {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Body received with GET, HEAD or OPTION method.");
			return ;
		}
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
			  "Normalize path to get proper file to serve." + eval_path);

	if (_method == METHOD_POST) {
		_log->log(LOG_DEBUG, RH_NAME,
				  "Path build to a POST request");
		_normalized_path = eval_path;
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
		_log->log(LOG_INFO, RH_NAME, "location size: " + int_to_string(_location->loc_default_pages.size()));
		for (size_t i = 0; i < _location->loc_default_pages.size(); i++) {
			_log->log(LOG_INFO, RH_NAME, "here" + eval_path + _location->loc_default_pages[i]);
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

//void HttpRequestHandler::validate_post_path() {
//	if (_content_type.empty()) {
//		turn_off_sanity(HTTP_BAD_REQUEST,
//						"Content-Type mandatory in POST Requests.");
//		return ;
//	}
//	if (!valid_mime_type(_path)) {
//		turn_off_sanity(HTTP_UNSUPPORTED_MEDIA_TYPE,
//						"MIME type not supported.");
//		return ;
//	}
//	if (black_list_extension(_path)) {
//		turn_off_sanity(HTTP_FORBIDDEN,
//						"Extension file is part of black list.");
//		return ;
//	}
//	std::string path_type = get_mime_type(_path);
//	if (path_type != _content_type) {
//		turn_off_sanity(HTTP_BAD_REQUEST,
//						"Content-Type header does not match with path POST request.");
//	}
//}

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
	                                      _normalized_path, _access, _sanity,
	                                      _status, _content_leght, _content_type, _boundary);
	HttpResponseHandler response(_location, _log, _client_data, request_wrapper, _fd);
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
