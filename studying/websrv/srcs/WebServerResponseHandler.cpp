/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServerResponseHandler.cpp                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/07 09:37:41 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/08 13:21:31 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#include "WebServerResponseHandler.hpp"

WsResponseHandler::WsResponseHandler(const LocationConfig *location,
									 const Logger *log,
									 ClientData* client_data,
									 s_request& request,
									 int fd):
									_fd(fd),
									_location(location),
									_log(log),
									_client_data(client_data),
									_request(request)
{
	if (_log == NULL)
		throw Logger::NoLoggerPointer();
}

WsResponseHandler::~WsResponseHandler(){}

bool WsResponseHandler::handle_request() {

	if (!_request.sanity) {
		send_error_response();
		return (false);
	}
	switch (_request.method) {
		case METHOD_GET:
			_log->log(LOG_DEBUG, RSP_NAME,
					  "Handle GET request.");
			return (handle_get());
		case METHOD_POST:
			_log->log(LOG_DEBUG, RSP_NAME,
					  "Handle POST request.");
			return 	(handle_post());
		case METHOD_DELETE:
			_log->log(LOG_DEBUG, RSP_NAME,
					  "Handle DELETE request.");
			return (handle_delete());
		default:
			turn_off_sanity(HTTP_NOT_IMPLEMENTED,
							"Method not allowed.");
			return (send_error_response());
	}
}

bool WsResponseHandler::handle_get() {
	if (_request.status != HTTP_OK || _request.access < ACCESS_READ) {
		send_error_response();
		return (false);
	}
	get_file_content(_request.normalized_path);
	if (_response_data.status) {
		_log->log(LOG_DEBUG, RSP_NAME,
				  "File content will be sent.");
		return (send_response(_response_data.content, _request.normalized_path));
	}
	_log->log(LOG_DEBUG, RSP_NAME,
			  "Get will send a error due to content load fails.");
	return (send_error_response());
}

void WsResponseHandler::get_file_content(std::string& path) {
	std::string content;

	if (path.empty()) {
		_response_data.status = false;
		return ;
	}
	try {
		std::ifstream file(path.c_str(), std::ios::binary);

		if (!file) {
			turn_off_sanity(HTTP_FORBIDDEN,
							"Fail to open file " + path);
			_request.status = HTTP_FORBIDDEN;
			return ;
		}
		file.seekg(0, std::ios::end);
		std::streampos file_size = file.tellg();
		file.seekg(0, std::ios::beg);
		content.resize(file_size);

		file.read(&content[0], file_size);

		if (!file) {
			_log->log(LOG_ERROR, RSP_NAME,
					  "Error reading file: " + path);
			_request.status = HTTP_INTERNAL_SERVER_ERROR;
			_response_data.status = false;
			return ;
		}
		file.close();
	} catch (const std::ios_base::failure& e) {
		_log->log(LOG_ERROR, RSP_NAME,
				  "I/O failure: " + std::string(e.what()));
		_request.status = HTTP_INTERNAL_SERVER_ERROR;
		_response_data.status = false;
		return ;
	} catch (const std::exception& e) {
		_log->log(LOG_ERROR, RSP_NAME,
				  "Exception: " + std::string(e.what()));
		_request.status = HTTP_INTERNAL_SERVER_ERROR;
		_response_data.status = false;
		return ;
	}
	_log->log(LOG_DEBUG, RSP_NAME,
			  "File content read OK.");
	_response_data.content = content;
	_response_data.status = true;
}

std::string WsResponseHandler::header(int code, size_t content_size, std::string mime) {
	std::ostringstream header;
	std::ostringstream connection;
	if (_client_data->is_active() && _request.sanity) {
		connection << "Connection: keep-alive\r\n"
				   << "Keep-Alive: timeout=" << TIMEOUT_CLIENT << "\r\n";
	} else {
		connection << "Connection: close\r\n";
	}
	std::ostringstream ranged;
	if (_response_data.ranged && _request.sanity) {
		ranged  << "Content-Range: bytes " << _response_data.start << "-" << _response_data.end
				<< "/" << _response_data.filesize << "\r\n"
				<< "Accept-Ranges: bytes\r\n";
	}
	header << "HTTP/1.1 " << code << " " << http_status_description((e_http_sts)code) << "\r\n"
		   << "Content-Length: " << content_size << "\r\n"
		   << "Content-Type: " <<  mime << "\r\n"
		   << connection.str()
		   << ranged.str()
		   << "\r\n";
	_log->log(LOG_DEBUG, RSP_NAME, header.str());
	return (header.str());
}

std::string WsResponseHandler::default_plain_error() {
	std::ostringstream content;
	content << "<!DOCTYPE html>\n"
			<< "<html>\n<head>\n"
			<< "<title>Webserver - Error</title>\n"
			<< "</head>\n<body>\n"
			<< "<h1>Something went wrong...</h1>\n"
			<< "<h2>" << int_to_string(_request.status) << " - "
			<< http_status_description(_request.status) << "</h2>\n"
			<< "</body>\n</html>\n";
	_log->log(LOG_DEBUG, RSP_NAME, "Build default error page.");
	return (content.str());
}

bool WsResponseHandler::send_error_response() {
	std::string error_file;
	std::string file_path = ".html";

	if (_location)
	{
		std::map<int, std::string>::const_iterator it;
		const std::map<int, std::string>* error_pages = &_location->loc_error_pages;
		if (_location->loc_error_mode == TEMPLATE) {
			it = error_pages->find(0);
		} else {
			it = error_pages->find(_request.status);
		}
		if (!error_pages->empty() || it != error_pages->end()) {
			get_file_content(error_file);
			if (!_response_data.status) {
				_log->log(LOG_DEBUG, RSP_NAME,
						  "Custom error page cannot be load. http status: " + int_to_string(_request.status));
				_response_data.content = default_plain_error();
			} else {
				_log->log(LOG_DEBUG, RSP_NAME, "Custom error page found.");
				file_path = error_file;
				if (_location->loc_error_mode == TEMPLATE)
				{
					_response_data.content = replace_template(_response_data.content, "{error_code}",
															  int_to_string(_request.status));
					_response_data.content = replace_template(_response_data.content, "{error_detail}",
															  http_status_description(_request.status));
					_log->log(LOG_DEBUG, RSP_NAME, "Template update to send.");
				}
			}
		} else {
			_log->log(LOG_DEBUG, RSP_NAME,
					  "Custom error page was not found. Error: " + int_to_string(_request.status));
		}
	}
	_headers = header(_request.status,
					  _response_data.content.length(),
					  get_mime_type(file_path));
	return (sender(_response_data.content));
}

bool WsResponseHandler::send_response(const std::string &body, const std::string &path) {
	std::string mime_type;
	if (_response_data.mime.empty()) {
		mime_type = get_mime_type(path);
	} else {
		mime_type = _response_data.mime;
	}
	_headers = header(_request.status, _response_data.content.length(), mime_type);
	return(sender(body));
}

bool WsResponseHandler::sender(const std::string& body) {
	try {
		std::string response = _headers + body;
		int total_sent = 0;
		int to_send = (int)response.length();
		while (total_sent < to_send) {
			int sent_bytes = (int)send(_fd, response.c_str() + total_sent, to_send - total_sent, 0);
			if (sent_bytes == -1) {
				_log->log(LOG_DEBUG, RSP_NAME,
						  "Error sending the response.");
				return (false);
			}
			total_sent += sent_bytes;
		}
		_log->log(LOG_DEBUG, RSP_NAME, "Response was sent.\n" + response);
		return (true);
	} catch(const std::exception& e) {
		_log->log(LOG_ERROR, RSP_NAME, e.what());
		return (false);
	}
}

bool WsResponseHandler::validate_payload(){
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
	if (black_list_extension(_request.normalized_path)) {
		turn_off_sanity(HTTP_UNSUPPORTED_MEDIA_TYPE,
		                "File extension is on black-list.");
		return (false);
	}
	return (true);
}

bool WsResponseHandler::handle_post() {
	if (_location->loc_access < ACCESS_WRITE) {
		turn_off_sanity(HTTP_FORBIDDEN,
						"No post data available.");
		return (send_error_response());
	}
	validate_payload();
	_client_data->chronos_reset();
	if (!_request.sanity) {
		return (send_error_response());
	}
	if (!save_file(_request.normalized_path, _request.body)) {
		return (send_error_response());
	}
	send_response("Created", _request.normalized_path);
	return (true);
}

bool WsResponseHandler::save_file(const std::string &save_path, const std::string& content) {
	try {
		std::ifstream check_file(save_path.c_str());
		if (check_file.good()) {
			turn_off_sanity(HTTP_CONFLICT,
			                "File already exists and cannot be overwritten.");
			check_file.close();
			return (false);
		}
		std::ofstream file(save_path.c_str());
		if (!file.is_open()) {
			turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
			                "Unable to open file to write.");
			return (false);
		}
		file << content;
		file.close();
		return (true);
	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Unable to open file to write due to unexpected error: " << e.what();
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
		                detail.str());
		return (false);
	}
}

bool WsResponseHandler::handle_delete() {
	if (_location->loc_access < ACCESS_DELETE) {
		turn_off_sanity(HTTP_FORBIDDEN,
						"Insufficient permissions to delete the resource.");
		return (send_error_response());
	}

	std::string delete_path = _request.normalized_path;

	if (!std::ifstream(delete_path.c_str()).good()) {
		turn_off_sanity(HTTP_NOT_FOUND,
						"Resource not found for deletion.");
		return (send_error_response());
	}

	if (std::remove(delete_path.c_str()) != 0) {
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
						"Failed to delete the resource.");
		return (send_error_response());
	}

	_request.status = HTTP_NO_CONTENT;
	_log->log(LOG_DEBUG, RSP_NAME,
			  "Resource deleted successfully: " + delete_path);
	send_response("Resource Deleted.", delete_path);
	return (true);
}

void WsResponseHandler::turn_off_sanity(e_http_sts status, std::string detail) {
	_log->log(LOG_ERROR, RSP_NAME, detail);
	_request.sanity = false;
	_request.status = status;
	_response_data.status = false;
}
