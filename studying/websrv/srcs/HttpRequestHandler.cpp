/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/10 01:17:31 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequestHandler.hpp"


HttpRequestHandler::HttpRequestHandler(const Logger* log,
									   ClientData* client_data,
									   WebServerCache* cache):
	_config(client_data->get_server()->get_config()),
	_log(log),
	_client_data(client_data),
	_cache(cache),
	_location(NULL),
	_fd(_client_data->get_fd().fd),
	_max_request(MAX_REQUEST) {
	if (!log) {
		throw Logger::NoLoggerPointer();
	}
	if (!client_data || !cache) {
		throw WebServerException("Some pointers are not valid. Server health is compromised.");
	}
	_request_data.sanity = true;
	_request_data.chunks = false;
	_request_data.cgi = false;
	_request_data.access = ACCESS_BAD_REQUEST;
	_factory = 0;
}

HttpRequestHandler::~HttpRequestHandler() {
	_log->log(LOG_DEBUG, RH_NAME,
	          "HttpRequestHandler resources clean up.");
}

void HttpRequestHandler::request_workflow() {
	validate_step steps[] = {&HttpRequestHandler::read_request_header,
	                         &HttpRequestHandler::parse_header,
	                         &HttpRequestHandler::parse_method_and_path,
	                         &HttpRequestHandler::parse_path_type,
	                         &HttpRequestHandler::load_header_data,
	                         &HttpRequestHandler::get_location_config,
	                         &HttpRequestHandler::cgi_normalize_path,
	                         &HttpRequestHandler::normalize_request_path,
	                         &HttpRequestHandler::load_content,
	                         &HttpRequestHandler::validate_request};

	_log->log(LOG_DEBUG, RH_NAME,
	          "Parse and Validation Request Process. Start");
	size_t i = 0;
	_client_data->chronos_reset();
	while (i < (sizeof(steps) / sizeof(validate_step)))
	{
		(this->*steps[i])();
		if (!_request_data.sanity)
			break;
		i++;
	}
	_log->log(LOG_DEBUG, RH_NAME,
	          "Request Validation Process. End. Send to Response Handler.");
	handle_request();
	_log->log(LOG_DEBUG, RH_NAME,
	          "Response Process end.");
}

void HttpRequestHandler::read_request_header() {
	char buffer[BUFFER_REQUEST];
	int         read_byte;
	size_t      size = 0;

	try {
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
			if (!_client_data->chronos_request()) {
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
			_client_data->deactivate();
			turn_off_sanity(HTTP_CLIENT_CLOSE_REQUEST,
			                "Client Close Request");
			return ;
		}
		_client_data->chronos_reset();
		_log->log(LOG_DEBUG, RH_NAME,
		          "Request read.");
	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Error Getting Header Data Request: " << e.what();
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR, detail.str());
	}
}


void HttpRequestHandler::parse_header() {
	size_t header_end = _request.find("\r\n\r\n");

	if (header_end != std::string::npos) {
		_request_data.header = _request.substr(0, header_end);
		if (_request_data.header.empty()) {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Request Header is empty.");
			return ;
		}
		_log->log(LOG_DEBUG, RH_NAME,
		          "Header successfully parsed.");
		header_end += 4;
		std::string tmp = _request.substr(header_end);
		_request.clear();
		_request = tmp;
	} else {
		turn_off_sanity(HTTP_BAD_REQUEST,
		                "Request parsing error: No header-body delimiter found.");
		return;
	}
}

void HttpRequestHandler::parse_method_and_path() {
	std::string method;
	std::string path;

	_log->log(LOG_DEBUG, RH_NAME,
			  "Parsing Request to get path and method.");
	size_t method_end = _request_data.header.find(' ');
	if (method_end != std::string::npos) {
		method = _request_data.header.substr(0, method_end);
		if (method.empty() || (_request_data.method = method_string_to_enum(method)) == METHOD_ERR ) {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Error parsing request: Method is empty or not valid.");
			return ;
		}

		size_t path_end = _request_data.header.find(' ', method_end + 1);
		if (path_end != std::string::npos) {
			path = _request_data.header.substr(method_end + 1, path_end - method_end - 1);
			if (path.size() > URI_MAX) {
				turn_off_sanity(HTTP_URI_TOO_LONG,
				                "Request path too long.");
				return ;
			}
			_log->log(LOG_DEBUG, RH_NAME,
			          "Request header fully parsed.");
			_request_data.path = path;
			return ;
		} else {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Error parsing request: missing path.");
			return ;
		}
	} else {
		turn_off_sanity(HTTP_BAD_REQUEST,
		                "Error parsing request: method malformed.");
		_request_data.method = METHOD_ERR;
		return ;
	}
}


void HttpRequestHandler::parse_path_type() {
	_log->log(LOG_DEBUG, RH_NAME,
			  "Parsing Path type.");
	size_t pos = _request_data.path.find('?');
	if (pos == std::string::npos) {
		_request_data.path_type = PATH_REGULAR;
		_log->log(LOG_DEBUG, RH_NAME,
				  "Regular Path to normalize.");
		return;
	}
	_request_data.query = _request_data.path.substr(pos + 1);
	_request_data.path = _request_data.path.substr(0, pos);
	_request_data.path_type = PATH_QUERY;
	_log->log(LOG_DEBUG, RH_NAME,
			  "Query found and parse from path.");
}


void HttpRequestHandler::load_header_data() {
	_log->log(LOG_DEBUG, RH_NAME, _request_data.header);
	std::string content_length = get_header_value(_request_data.header,
												  "content-length:", "\r\n");
	_request_data.content_type = get_header_value(_request_data.header,
												  "content-type:", "\r\n");

	if (!content_length.empty()){
		if (is_valid_size_t(content_length)) {
			_request_data.content_length = str_to_size_t(content_length);
		} else {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Content-Length malformed.");
		}
	} else {
		_request_data.content_length = 0;
	}
	if (!_request_data.content_type.empty()) {
		if (starts_with(_request_data.content_type, "multipart")) {
			_request_data.boundary = get_header_value(_request_data.content_type,
													  "boundary", "\r\n");
			if (_request_data.boundary.empty()) {
				turn_off_sanity(HTTP_BAD_REQUEST,
				                "Boundary malformed or not present with a multipart Content-Type.");
			}
			_factory++;
			size_t end_type = _request_data.content_type.find(';');
			if (end_type != std::string::npos) {
				_request_data.content_type = _request_data.content_type.substr(0, end_type);
			}
		}
	}
	std::string chunks = get_header_value(_request_data.header,
										  "transfer-encoding:", "\r\n");
	_log->log(LOG_DEBUG, RSP_NAME, chunks);
	if (!chunks.empty()) {
		chunks = trim(chunks, " ");
		if (chunks == "chunked") {
			_request_data.chunks = true;
		}
	}
	_request_data.range = get_header_value(_request_data.header,
										   "range:", "\r\n");
	if (!_request_data.range.empty()) {
		_factory++;
	}
	std::string keep = get_header_value(_request_data.header,
										"connection:", "\r\n");
	if (keep == "keep-alive") {
		_client_data->keep_active();
	} else {
		_client_data->deactivate();
	}
	_request_data.cookie = get_header_value(_request_data.header,
											"cookie", "\r\n");
}


void HttpRequestHandler::get_location_config() {
	std::string saved_key;
	const LocationConfig* result = NULL;

	_log->log(LOG_DEBUG, RH_NAME,
			  "Searching related location.");
	for (std::map<std::string, LocationConfig>::const_iterator it = _config.locations.begin();
	     it != _config.locations.end(); ++it) {
		const std::string& key = it->first;
		if (starts_with(_request_data.path, key)) {
			if (key.length() > saved_key.length()) {
				result = &it->second;
				saved_key = key;
			}
		}
	}
	if (result) {
		_log->log(LOG_DEBUG, RH_NAME,
				  "Location Found: " + saved_key);
		_location = result;
		_request_data.access = result->loc_access;
	} else {
		turn_off_sanity(HTTP_BAD_REQUEST,
						"Location Not Found");
	}
}


void HttpRequestHandler::cgi_normalize_path() {
	if (!_location->cgi_file) {
		_log->log(LOG_DEBUG, RH_NAME,
		          "No CGI locations at server config.");
		return ;
	}
	std::string eval_path = _config.server_root + _request_data.path;
	if (is_dir(eval_path)) {
		_log->log(LOG_DEBUG, RH_NAME,
		          "CGI test - Directory exist.");
		return ;
	}
	if (eval_path[eval_path.size() - 1] != '/' && is_file(eval_path) && is_cgi(eval_path)) {
		_log->log(LOG_DEBUG, RH_NAME,
		          "The user is over a real CGI file. It should be handle as script.");
		size_t dot_pos = eval_path.find_last_of('/');
		_request_data.script = eval_path.substr(dot_pos + 1);
		_request_data.normalized_path = eval_path.substr(0, dot_pos);
		_request_data.cgi = true;
		_factory++;
		return ;
	}

	std::string saved_key;
	const t_cgi* cgi_data = NULL;

	_log->log(LOG_DEBUG, RH_NAME,
	          "Request will be testing against mapped CGI scripts.");
	for (std::map<std::string, t_cgi>::const_iterator it = _location->cgi_locations.begin();
	     it != _location->cgi_locations.end(); it++) {
		const std::string& key = it->first;
		if (starts_with(_request_data.path, key)) {
			if (key.length() > saved_key.length()) {
				cgi_data = &it->second;
				saved_key = key;
			}
		}
	}
	if (cgi_data) {
		_log->log(LOG_DEBUG, RH_NAME,
		          "CGI - Location Found: " + saved_key);
		_request_data.normalized_path = _config.server_root + cgi_data->cgi_path;
		_request_data.script = cgi_data->script;
		_request_data.path_info = _request_data.path.substr(saved_key.length());
		_request_data.cgi = true;
		_factory++;
	}
}


void HttpRequestHandler::normalize_request_path() {
	if (_request_data.cgi) {
		_log->log(LOG_DEBUG, RH_NAME,
		          "CGI context. path has been normalized");
		return ;
	}
	std::string eval_path = _config.server_root + _request_data.path;

	_log->log(LOG_DEBUG, RH_NAME,
			  "Normalize path to get proper file to serve." + eval_path);
	if (_request_data.method == METHOD_POST) {
		_log->log(LOG_DEBUG, RH_NAME,
				  "Path build to a POST request");
		if (eval_path[eval_path.size() - 1] != '/') {
			eval_path += "/";
		}
		if (!is_dir(eval_path)){
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Non valid path to create resource.");
			return ;
		}
		_request_data.normalized_path = eval_path;
		return ;
	}
	if (eval_path[eval_path.size() - 1] != '/' && is_file(eval_path)) {
		_log->log(LOG_INFO, RH_NAME, "File found.");
		_request_data.normalized_path = eval_path;
		_request_data.cgi = is_cgi(_request_data.normalized_path);
		_request_data.status = HTTP_OK;
		return ;
	}
	if (_request_data.method == METHOD_DELETE) {
		turn_off_sanity(HTTP_NOT_FOUND,
		                "Resource to be deleted, not found.");
		return ;
	}
	if (is_dir(eval_path)) {
		if (_request_data.method == METHOD_DELETE) {
			turn_off_sanity(HTTP_METHOD_NOT_ALLOWED,
							"Delete method over a dir is not allowed.");
			return ;
		}
		if (eval_path[eval_path.size() - 1] != '/') {
			eval_path += "/";
		}
		_log->log(LOG_INFO, RH_NAME, "location size: " + int_to_string(_location->loc_default_pages.size()));
		for (size_t i = 0; i < _location->loc_default_pages.size(); i++) {
			_log->log(LOG_INFO, RH_NAME, "here" + eval_path + _location->loc_default_pages[i]);
			if (is_file(eval_path + _location->loc_default_pages[i])) {
				_log->log(LOG_INFO, RH_NAME, "Default File found");
				_request_data.normalized_path = eval_path + _location->loc_default_pages[i];
				_request_data.cgi = is_cgi(_request_data.normalized_path);
				_request_data.status = HTTP_OK;
				return ;
			}
		}
	}
	turn_off_sanity(HTTP_NOT_FOUND,
	                "Requested path not found " + _request_data.path);
}


void HttpRequestHandler::load_content() {
	if (_request_data.chunks) {
		_log->log(LOG_DEBUG, RH_NAME,
				  "Chunk content. Load body request.");
		load_content_chunks();
	} else {
		_log->log(LOG_DEBUG, RH_NAME,
				  "Normal content. Load body request.");
		load_content_normal();
	}
}

void HttpRequestHandler::load_content_chunks() {
	char buffer[BUFFER_REQUEST];
	int read_byte;
	size_t size = 0;
	std::string chunk_data = _request;
	_request.clear();
	long chunk_size = 1;

	try {
		while (true) {
			if (!_client_data->chronos_request()) {
				turn_off_sanity(HTTP_REQUEST_TIMEOUT,
				                "Request Timeout.");
				return ;
			}
			parse_chunks(chunk_data, chunk_size);
			if (chunk_size == 0) {
				size = _request.length();
				break ;
			}
			read_byte = recv(_fd, buffer, sizeof(buffer), 0);
			if (read_byte <= 0) {
				continue ;
			} else {
				size += read_byte;
			}
			if (size > _max_request) {
				turn_off_sanity(HTTP_CONTENT_TOO_LARGE,
				                "Body Content too Large.");
				return ;
			}
			chunk_data.append(buffer, read_byte);
		}

		if (size == 0) {
			_client_data->deactivate();
			turn_off_sanity(HTTP_CLIENT_CLOSE_REQUEST,
			                "Client Close Request");
			return ;
		}
		if (read_byte < 0 && size == 0) {
			turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
			                "Error Reading From Socket.");
			return ;
		}
		_request_data.body = _request;
		_request_data.content_length = _request_data.body.length();
		_log->log(LOG_DEBUG, RH_NAME,
		          "Chunked Request read.");
	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Error getting chunk data: " << e.what();
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
						detail.str());
	}
}


bool HttpRequestHandler::parse_chunks(std::string& chunk_data, long& chunk_size) {
	size_t pos = 0;

	while (true) {
		size_t chunk_size_end = chunk_data.find("\r\n", pos);
		if (chunk_size_end == std::string::npos) {
			break ;
		}

		std::string chunk_size_str = chunk_data.substr(pos, chunk_size_end - pos);
		char* endptr;
		errno = 0;
		chunk_size = strtol(chunk_size_str.c_str(), &endptr, 16);

		if (errno == ERANGE || chunk_size < 0 || *endptr != '\0') {
			turn_off_sanity(HTTP_BAD_REQUEST,
							"Invalid chunk size format.");
			return (false);
		}

		pos = chunk_size_end + 2;
		//	TODO: Check if any char is left?
		if (chunk_size == 0) {
			if (chunk_data.size() < pos + 2) {
				break ;
			}
			if (chunk_data.compare(pos, 2, "\r\n") == 0) {
				chunk_data.erase(0, pos + 2);
				return (true);
			} else {
				turn_off_sanity(HTTP_BAD_REQUEST,
								"Invalid chunk ending.");
				return (false);
			}
		}
		if (chunk_data.size() < pos + chunk_size + 2) {
			break ;
		}
		_request.append(chunk_data, pos, chunk_size);
		pos += chunk_size + 2;
		if (_request.size() > _max_request) {
			turn_off_sanity(HTTP_CONTENT_TOO_LARGE,
							"Body Content too Large.");
			return (false);
		}
	}
	chunk_data.erase(0, pos);
	return (true);
}


void HttpRequestHandler::load_content_normal() {
	if (_request_data.content_length == 0) {
		_log->log(LOG_INFO, RH_NAME,
		          "No Content-Length to read from FD.");
		return ;
	}

	char buffer[BUFFER_REQUEST];
	int         read_byte;
	size_t      size = 0;
	size_t      to_read = _request_data.content_length - _request.length();

	try {
		while (to_read > 0) {
			read_byte = recv(_fd, buffer, sizeof(buffer), 0);
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
			if (!_client_data->chronos_request()) {
				turn_off_sanity(HTTP_REQUEST_TIMEOUT,
				                "Request Timeout.");
				return ;
			}
		}
		if (read_byte < 0 && size == 0) {
			turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
			                "Error Reading From Socket. ");
			return ;
		}
		if (size == 0) {
			_client_data->deactivate();
			turn_off_sanity(HTTP_CLIENT_CLOSE_REQUEST,
			                "Client Close Request");
			return ;
		}
		_client_data->chronos_reset();
		_request_data.body = _request;
		_log->log(LOG_DEBUG, RH_NAME,
				  "Request read.");
	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Error reading request: " << e.what();
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
						detail.str());
	}
}


void HttpRequestHandler::validate_request() {
	if (!_request_data.body.empty()) {
		if (_request_data.method == METHOD_GET
			|| _request_data.method == METHOD_HEAD
			|| _request_data.method == METHOD_OPTIONS) {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Body received with GET, HEAD or OPTION method.");
			return ;
		}
	} else {
		if (_request_data.method == METHOD_POST
			|| _request_data.method == METHOD_PUT
			|| _request_data.method == METHOD_PATCH) {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Body empty with POST, PUT or PATCH method.");
		}
	}
}


void HttpRequestHandler::handle_request() {
	if (!_client_data->is_active()) {
		return;
	}
	try {
		if (_factory == 0) {
			HttpResponseHandler response(_location, _log, _client_data, _request_data, _fd, _cache);
			response.handle_request();
			return ;
		}
		if (_request_data.cgi) {
			HttpCGIHandler response(_location, _log, _client_data, _request_data, _fd);
			response.handle_request();
			_client_data->deactivate();
			return ;
		} else if (!_request_data.range.empty()){
			HttpRangeHandler response(_location, _log, _client_data, _request_data, _fd);
			response.handle_request();
			return ;
		} else if (!_request_data.boundary.empty()){
			HttpMultipartHandler response(_location, _log, _client_data, _request_data, _fd);
			response.handle_request();
			return ;
		}
	} catch (WebServerException& e) {
		std::ostringstream detail;
		detail << "Error Handling response: " << e.what();
		_log->log(LOG_ERROR, RH_NAME,
				  detail.str());
		_client_data->deactivate();
	} catch (Logger::NoLoggerPointer& e) {
		std::ostringstream detail;
		detail << "Logger Pointer Error at Response Handler. "
			   << "Server Sanity could be compromise.";
		_log->log(LOG_ERROR, RH_NAME,
		          detail.str());
		_client_data->deactivate();
	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Unknown error handling response: " << e.what();
		_log->log(LOG_ERROR, RH_NAME,
		          detail.str());
		_client_data->deactivate();
	}
}

void HttpRequestHandler::turn_off_sanity(e_http_sts status, std::string detail) {
	_log->log(LOG_ERROR, RH_NAME, detail);
	_request_data.sanity = false;
	_request_data.status = status;
}
