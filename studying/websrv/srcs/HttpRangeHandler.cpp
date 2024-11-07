/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRangeHandler.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/07 09:37:41 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/07 09:37:41 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRangeHandler.hpp"

HttpRangeHandler::~HttpRangeHandler() {}

HttpRangeHandler::HttpRangeHandler(const LocationConfig *location,
								   const Logger *log,
								   ClientData *client_data,
								   s_request &request,
								   int fd) :
		WsResponseHandler(location, log,
		                  client_data, request,
		                  fd) {
	_log->log(LOG_DEBUG, RRH_NAME,
	          "Range Response Handler Init.");
}

bool HttpRangeHandler::handle_request() {

	switch (_request.method) {
		case METHOD_GET:
			_log->log(LOG_DEBUG, RRH_NAME,
			          "Handle GET request.");
			return (handle_get());
		default:
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Range petition only allowed with GET method.");
			send_error_response();
	}
	return (true);
}

bool HttpRangeHandler::handle_get() {
	if (_request.status != HTTP_OK || _request.access < ACCESS_READ) {
		send_error_response();
		return (false);
	}
	get_file_content(_request.normalized_path);
	if (_response_data.status) {
		_log->log(LOG_DEBUG, RSP_NAME,
		          "File content will be sent.");
		return (send_response(_response_data.content, _request.normalized_path));
	} else {
		_log->log(LOG_ERROR, RSP_NAME,
		          "Get will send a error due to content load fails.");
		return (send_error_response());
	}
}

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
	_log->log(LOG_DEBUG, RRH_NAME,
			  "File content read completed.");
}


void HttpRangeHandler::parse_content_range() {
	_response_data.ranged = false;
	_response_data.start = 0;
	_response_data.end = static_cast<size_t>(-1); // Inicia con un valor indicativo
	_log->log(LOG_DEBUG, RRH_NAME,
	          "Parsing Range header: " + _request.range);

	if (!_request.range.empty()) {
		std::string range_value = _request.range;
		unsigned long start = 0, end = static_cast<unsigned long>(-1);

		if (sscanf(range_value.c_str(), "bytes=%lu-%lu", &start, &end) == 2) {
			_response_data.start = start;
			_response_data.end = end;
			_response_data.ranged = true;
			_response_data.range_scenario = CR_RANGE;
		} else if (sscanf(range_value.c_str(), "bytes=%lu-", &start) == 1) {
			_response_data.start = start;
			_response_data.end = _response_data.start + DEFAULT_RANGE_BYTES;
			_response_data.ranged = true;
			_response_data.range_scenario = CR_INIT;
		} else if (sscanf(range_value.c_str(), "bytes=-%lu", &end) == 1) {
			_response_data.end = end;
			_response_data.ranged = true;
			_response_data.range_scenario = CR_LAST;
		} else {
			turn_off_sanity(HTTP_RANGE_NOT_SATISFIABLE,
							"Malformed Range Header.");
			return;
		}
	} else {
		_log->log(LOG_DEBUG, RRH_NAME,
				  "No Range header found.");
	}
	_log->log(LOG_DEBUG, RRH_NAME,
	          "Parsed ranged.");
}

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
	}
	_log->log(LOG_DEBUG, RRH_NAME,
	          "Validated content range.");
	return (true);
}

void HttpRangeHandler::get_file_content(int pid, int (&fd)[2]) {
	UNUSED(pid);
	UNUSED(fd);
}
