/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpMultipartHandler.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/10 20:43:26 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/15 14:44:59 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpMultipartHandler.hpp"

/**
 * @brief Constructs an `HttpMultipartHandler` for handling multipart HTTP requests.
 *
 * Initializes the handler with the specified location configuration, logger, client data,
 * and file descriptor. Logs the initialization process.
 *
 * @param location Pointer to the `LocationConfig` object containing server configuration details.
 * @param log Pointer to the `Logger` instance for logging.
 * @param client_data Pointer to `ClientData` with details of the client connection.
 * @param request Reference to `s_request` with the incoming request information.
 * @param fd File descriptor associated with the client connection.
 *
 */
HttpMultipartHandler::HttpMultipartHandler(const LocationConfig *location,
										   const Logger *log,
										   ClientData *client_data,
										   s_request &request,
										   int fd) :
										   WsResponseHandler(location, log,
															 client_data, request,
															 fd) {
	_log->log_debug( MP_NAME,
	          "Multipart Handler init.");
}

/**
 * @brief Processes the HTTP request based on the method type.
 *
 * This function validates the request method and if the method
 * is `POST`, it forwards the request to the `handle_post` method.
 *
 * @return `true` if the request was successfully processed, `false` otherwise.
 *
 * @details
 * - **Supported Method**: Only `POST` method is allowed; other methods will trigger an error response.
 */
bool HttpMultipartHandler::handle_request() {

	switch (_request.method) {
		case METHOD_POST:
			_log->log_debug( MP_NAME,
			          "Handle POST request.");
			return (handle_post());
		default:
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Multipart only allowed to post method.");
			return (send_error_response());
	}
}

/**
 * @brief Handles the `POST` method for multipart form-data by saving each part of the content.
 *
 * This function checks permissions, validates the request payload, and saves uploaded files
 * or data in the specified directory.
 *
 * @return `true` if all parts are saved successfully; `false` if there are errors.
 *
 * @details
 * - **Permission Check**: Ensures the current location has `WRITE` access.
 * - **Payload Validation**: Verifies request content length and type.
 * - **File Saving**: Iterates through `_multi_content` to save each file part.
 */
bool HttpMultipartHandler::handle_post() {
	if (!HAS_POST(_location->loc_allowed_methods)) {
		turn_off_sanity(HTTP_FORBIDDEN,
		                "No post data available due permissions.");
		return (send_error_response());
	}
	if (!validate_payload()) {
		return (send_error_response());
	}
	std::string save_path;
	for (size_t i = 0; i < _multi_content.size(); i++) {
		if (_multi_content[i].data_type == CT_FILE) {
			save_path = _multi_content[1].filename;
			if (!_request.boundary.empty()) {
				save_path = _request.normalized_path + _multi_content[i].filename;
			}
			if (!save_file(save_path, _multi_content[i].data)) {
				if (i > 0) {
					turn_off_sanity(HTTP_MULTI_STATUS,
									"Error posting some resources.");
					return (send_response("Partially Created.",
										  _request.normalized_path));
				} else {
					return (send_error_response());
				}
			} else {
				_request.status = HTTP_CREATED;
				_log->log_debug( MP_NAME,
				          "File Data Recieved and saved.");
			}
		}
	}
	return(send_response("Created", _request.normalized_path));
}

/**
 * @brief Validates the payload of a multipart `POST` request.
 *
 * This function checks the request's content length, type, and body for compliance with
 * multipart requirements. If any validation fails, it deactivates the request sanity.
 *
 * @return `true` if the payload is valid; `false` otherwise.
 *
 * @details
 * - **Content Length**: Ensures that `content_length` is non-zero.
 * - **Content Type**: Checks for a valid `Content-Type` header.
 * - **Boundary**: Verifies the presence of the boundary in multipart data.
 */
bool HttpMultipartHandler::validate_payload() {
	if (!_request.sanity) {
		return (false);
	}
	if (_request.content_length == 0) {
		turn_off_sanity(HTTP_LENGTH_REQUIRED,
		                "Content-length required at POST request.");
		return (false);
	}
	if (_request.content_type.empty()) {
		turn_off_sanity(HTTP_BAD_REQUEST,
		                "Content-Type required at POST request.");
		return (false);
	}
	if (_request.body.empty()) {
		turn_off_sanity(HTTP_BAD_REQUEST,
		                "No body request required at POST request.");
		return (false);
	} else {
		if (_request.body.length() != _request.content_length) {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Received content-length and content size does not match.");
			return (false);
		}
	}
	parse_multipart_data();
	for (size_t i = 0; i < _multi_content.size(); i++) {
		if (!_multi_content[i].filename.empty() && black_list_extension(_multi_content[i].filename)) {
			turn_off_sanity(HTTP_UNSUPPORTED_MEDIA_TYPE,
			                "File extension is on black-list.");
			return (false);
		}
	}
	return (true);
}

/**
 * @brief Parses the multipart data in the request body, extracting each part.
 *
 * This function identifies and extracts individual parts from the multipart payload,
 * storing them in `_multi_content`. Each part includes metadata like `filename`, `content-type`,
 * and actual content data.
 *
 * @details
 * - **Part Extraction**: Uses `_request.boundary` to identify separate parts.
 * - **Error Handling**: Ensures complete data for each part; otherwise, it disables sanity.
 */
void HttpMultipartHandler::parse_multipart_data() {
	size_t part_start = _request.body.find(_request.boundary);
	while (part_start != std::string::npos) {
		part_start += _request.boundary.length() + 2;
		size_t headers_end = _request.body.find("\r\n\r\n", part_start);
		if (headers_end != std::string::npos) {
			_log->log_debug( MP_NAME,
			          "Looking for Post multi-part related data.");
			std::string content_info = _request.body.substr(part_start, headers_end - part_start);
			size_t file_data_start = headers_end + 4;
			size_t next_boundary = _request.body.find(_request.boundary, file_data_start);
			std::string content_data = _request.body.substr(file_data_start, next_boundary - file_data_start);
			if (content_info.empty() || content_data.empty()) {
				turn_off_sanity(HTTP_BAD_REQUEST,
				                "Multi-part body request is incomplete.");
				return ;
			}
			std::string content_disposition = get_header_value(content_info, "content-disposition:", "\r\n");
			s_multi_part part_data(get_header_value(content_info, "content-disposition:", ";"),
			                       trim(get_header_value(content_disposition, "name", ";"), "\""),
			                       trim(get_header_value(content_disposition, "filename", ";"), "\""),
			                       get_header_value(content_info, "content-type:", "\r\n"),
			                       content_data);
			if (part_data.filename.empty()) {
				part_data.data_type = CT_UNKNOWN;
			}
			_multi_content.push_back(part_data);
			part_start = next_boundary;
		} else {
			_log->log_debug( MP_NAME,
			          "End of multi-part body request.");
			break;
		}
	}
}

/**
 * @brief Placeholder for pure abstract method
 *
 * @param pid Process ID (unused).
 * @param fd File descriptor array (unused).
 */
void HttpMultipartHandler::get_file_content(int pid, int (&fd)[2]) {
	UNUSED(pid);
	UNUSED(fd);
}
