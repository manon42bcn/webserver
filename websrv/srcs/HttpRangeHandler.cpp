/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRangeHandler.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/07 09:37:41 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/15 10:56:24 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRangeHandler.hpp"

/**
 * @brief Constructs an `HttpRangeHandler` for handling HTTP range requests.
 *
 * Initializes the `HttpRangeHandler` by passing the location configuration, logger, client data,
 * request structure, and file descriptor to the base class, `WsResponseHandler`. Logs the
 * initialization process to facilitate tracing.
 *
 * @param location Pointer to `LocationConfig`, providing server configuration settings for the requested location.
 * @param log Pointer to the `Logger` instance for logging operations.
 * @param client_data Pointer to `ClientData`, containing information about the client connection.
 * @param request Reference to an `s_request` structure holding parsed HTTP request data.
 * @param fd File descriptor used for managing the response output.
 *
 * @note Assumes that `location`, `log`, and `client_data` are valid pointers.
 */
HttpRangeHandler::HttpRangeHandler(const LocationConfig *location,
								   const Logger *log,
								   ClientData *client_data,
								   s_request &request,
								   int fd) :
		WsResponseHandler(location, log,
		                  client_data, request,
		                  fd) {
	_log->log_debug( RRH_NAME,
	          "Range Response Handler Init.");
}

/**
 * @brief Processes the HTTP range request, allowing only GET requests.
 *
 * This method handles HTTP range requests, verifying that the request method is `GET`.
 * If the method is `GET`, it proceeds with `handle_get()` to manage the range response.
 * For any other method, it disables sanity, logs an error, and sends an HTTP bad request response.
 *
 * @return `true` if the request was processed as a GET range request; `false` otherwise.
 *
 * @note Only `GET` requests are supported for range handling.
 * 	     Other methods will result in a `400 Bad Request`.
 */
bool HttpRangeHandler::handle_request() {

	switch (_request.method) {
		case METHOD_GET:
			_log->log_debug( RRH_NAME,
			          "Handle GET request.");
			return (handle_get());
		default:
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Range petition only allowed with GET method.");
			send_error_response();
	}
	return (true);
}

/**
 * @brief Processes the GET HTTP range requests.
 *
 * This methods overloads handle_get just to be able to handle different HTTP status
 * depending of the range. HTTP PARTIAL CONTENT is important to render ranged contents.
 *
 * @return `true` if the request was processed as a GET range request; `false` otherwise.
 *
 * @note Full content will return 200 HTTP OK, Other ranges HTTP PARTIAL CONTENT.
 */
bool HttpRangeHandler::handle_get() {
	if (!HAS_GET(_location->loc_allowed_methods)) {
		turn_off_sanity(HTTP_FORBIDDEN,
						"Get not allowed over resource.");
		send_error_response();
		return (false);
	}
	get_file_content(_request.normalized_path);
	if (_response_data.status) {
		_log->log_debug( RSP_NAME,
						 "File content will be sent.");
		return (send_response(_response_data.content, _request.normalized_path));
	}
	_log->log_debug( RSP_NAME,
					 "Get will send a error due to content load fails.");
	return (send_error_response());
}

/**
 * @brief Reads file content within the specified range, if valid, and stores it in `_response_data`.
 *
 * This method retrieves file content based on a specified range, ensuring the range is valid and within
 * the file's size. If any error occurs (such as missing permissions, invalid range, or read failure),
 * it disables sanity and logs an appropriate error message.
 *
 * @param path Reference to the file path as a string.
 *
 * @details
 * - Calls `validate_content_range` to check the validity of the specified content range.
 * - Reads the content within the specified range if it exists and assigns it to `_response_data.content`.
 * - Handles file access exceptions and I/O errors, logging any issues encountered during the process.
 *
 * @note If the content range is invalid, `_response_data.status` is set to `false`, and no content is read.
 */
void HttpRangeHandler::get_file_content(std::string& path) {
	if (path.empty()) {
		turn_off_sanity(HTTP_BAD_REQUEST,
		                "Path is needed.");
		return;
	}
	try {
		std::string content;
		parse_content_range();
		_response_data.status = false;
		std::ifstream file(path.c_str(), std::ios::binary);
		if (!file) {
			turn_off_sanity(HTTP_FORBIDDEN,
							"Failed to open file " + path);
			return;
		}
		file.seekg(0, std::ios::end);
		std::streampos file_size = file.tellg();
		_response_data.filesize = file_size;
		file.seekg(0, std::ios::beg);

		if (validate_content_range(file_size)) {
			size_t content_length = _response_data.end - _response_data.start + 1;
			content.resize(content_length, '\0');
			file.seekg(_response_data.start);
			file.read(&content[0], content_length);
			if (!file) {
				turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
				                "Error reading file: " + path);
				file.close();
				_response_data.status = false;
				return ;
			}
			_response_data.status = true;
			_request.status = HTTP_PARTIAL_CONTENT;
		}

		file.close();
		_response_data.content = content;
	} catch (const std::ios_base::failure& e) {
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
						"I/O failure: " + std::string(e.what()));
	} catch (const std::exception& e) {
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
						"Unhandled Exception: " + std::string(e.what()));
	}
	_log->log_debug( RRH_NAME,
			  "File content read completed.");
}

/**
 * @brief Parses the Range header in an HTTP request to extract byte range values.
 *
 * This method interprets the `Range` header, identifying and storing the start and end
 * byte positions in `_response_data` based on the type of range specified.
 * It handles scenarios such as a full byte range, a range from the start position,
 * and a range from the last specified bytes.
 *
 * @details
 * - If the header format is valid, `_response_data.ranged` is set to `true`, and the corresponding
 *   start and end positions are populated.
 * - In the case of an invalid format, the method disables sanity with a `416 Range Not Satisfiable` status.
 *
 * @note `_response_data.ranged` will be `false` if the range header is malformed.
 */
void HttpRangeHandler::parse_content_range() {
	_response_data.ranged = false;
	_response_data.start = 0;
	_response_data.end = static_cast<size_t>(-1);
	_log->log_debug( RRH_NAME,
	          "Parsing Range header: " + _request.range);

	std::string range_value = _request.range;
	unsigned long start = 0, end = static_cast<unsigned long>(-1);

	if (sscanf(range_value.c_str(), "bytes=%lu-%lu", &start, &end) == 2) {
		_response_data.start = start;
		_response_data.end = end;
		_response_data.ranged = true;
		_response_data.range_scenario = CR_RANGE;
		_log->log_debug( RRH_NAME,
		          "Full range found");
	} else if (sscanf(range_value.c_str(), "bytes=%lu-", &start) == 1) {
		_response_data.start = start;
		_response_data.end = _response_data.start + DEFAULT_RANGE_BYTES;
		_response_data.ranged = true;
		_response_data.range_scenario = CR_INIT;
		_log->log_debug( RRH_NAME,
		          "Partial Range: start-");
	} else if (sscanf(range_value.c_str(), "bytes=-%lu", &end) == 1) {
		_response_data.end = end;
		_response_data.ranged = true;
		_response_data.range_scenario = CR_LAST;
		_log->log_debug( RRH_NAME,
		          "Last Range found.");
	} else {
		turn_off_sanity(HTTP_RANGE_NOT_SATISFIABLE,
						"Malformed Range Header.");
		_response_data.ranged = false;
		return;
	}
}

/**
 * @brief Validates and adjusts the content range within the file size.
 *
 * This method checks if the requested range is valid based on the file size, adjusting the start and end
 * positions as necessary. If the range is not satisfiable, it disables sanity and returns false.
 *
 * @param file_size The total size of the file in bytes.
 * @return `true` if the range is valid and satisfiable, `false` otherwise.
 *
 * @details
 * - For scenarios such as `CR_LAST`, the range is adjusted from the end of the file backwards.
 * - If the range is larger than the file, it is capped to the file's size.
 * - If `end` equals the file size - 1, `_request.status` is set to `HTTP_OK` for full file requests.
 *
 * @note Logs the final validated range for debugging.
 */
bool HttpRangeHandler::validate_content_range(size_t file_size) {
	if (_response_data.range_scenario == CR_LAST) {
		if (file_size > _response_data.end) {
			_response_data.start = file_size - _response_data.end;
		} else {
			_response_data.start = 0;
		}
		_response_data.end = file_size - 1;
	} else if (_response_data.range_scenario == CR_INIT) {
		_response_data.end = std::min(_response_data.end, file_size - 1);
	} else if (_response_data.range_scenario == CR_RANGE) {
		if (_response_data.end >= file_size) {
			_response_data.end = file_size - 1;
		}
	}
	if (_response_data.start >= file_size || _response_data.start > _response_data.end) {
		turn_off_sanity(HTTP_RANGE_NOT_SATISFIABLE,
						"Requested range is not satisfiable.");
		return (false);
	}
	if (_response_data.end == file_size) {
		_request.status = HTTP_OK;
		_client_data->deactivate();
	}
	_log->log_debug( RRH_NAME,
	          "Validated content range.");
	return (true);
}

/**
 * @brief Placeholder for retrieving file content using process ID and file descriptor array.
 *
 * @note Unused function
 */
void HttpRangeHandler::get_file_content(int pid, int (&fd)[2]) {
	UNUSED(pid);
	UNUSED(fd);
}
