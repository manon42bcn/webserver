/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/30 16:24:30 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequestHandler.hpp"

/**
 * @brief Constructs an HttpRequestHandler to process and validate an HTTP request.
 *
 * This constructor initializes the request handler with the logger and client data,
 * then proceeds through a series of validation and processing steps.
 * If any validation step fails or a CGI flag is set, it stops further processing.
 *
 * @details
 * - First validates the logger pointer and throws an exception if it's null.
 * - It then performs a series of validation steps in order:
 *   1. `read_request_header()`
 *   2. `parse_header()`
 *   3. `parse_method_and_path()`
 *   4. `parse_path_type()`
 *   5. `load_header_data()`
 *   6. `get_location_config()`
 *   7. `cgi_normalize_path()`
 *   8. `normalize_request_path()`
 *   9. `load_content()`
 *   10. `validate_request()`
 * - If any step fails (i.e., `_sanity` is false), the process halts.
 * - Upon completion, it calls `handle_request()` to handle the validated request.
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
	_sanity(true),
	_cgi(false) {
	if (_log == NULL) {
		throw Logger::NoLoggerPointer();
	}
	_chunks = false;
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

/**
 * @brief Destructor for HttpRequestHandler, handles resource cleanup.
 *
 * This destructor ensures that any pointers are set to `NULL` after use, signaling
 * that they are no longer valid. Logs the cleanup process for tracking purposes.
 *
 * @details
 * - Logs the destructor call and resource cleanup for traceability.
 * - Sets `_client_data`, `_location`, and `_log` to `NULL` for safety, even though no
 *   explicit memory deallocation is needed.
 *
 * @return None
 */
HttpRequestHandler::~HttpRequestHandler() {
	_log->log(LOG_DEBUG, RH_NAME,
	          "HttpRequestHandler resources clean up.");
	_client_data = NULL;
	_location = NULL;
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
// TODO: This function and next can be merged using getline
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
//	_all_headers = parse_headers_map(_header);
	_log->log(LOG_DEBUG, RH_NAME, _request);
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

	_log->log(LOG_DEBUG, RH_NAME,
			  "Parsing Request to get path and method.");
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
 * @brief Parses the path to determine if it contains a query component.
 *
 * This method checks if `_path` contains a query string, identified by the presence of
 * a `?` character. If a query is found, it separates the query from the main path and
 * updates `_path_type` to `PATH_QUERY`. Otherwise, `_path_type` is set to `PATH_REGULAR`.
 *
 * @details
 * - If a `?` is found in `_path`, the substring after `?` is extracted and assigned to `_query_string`.
 * - The main path up to `?` is retained in `_path`, and `_path_type` is set to `PATH_QUERY`.
 * - If no `?` is found, `_path_type` is set to `PATH_REGULAR`, indicating no query is present.
 * - Logs the detected path type and, if applicable, the query string for debugging.
 *
 * @return None
 */
void HttpRequestHandler::parse_path_type() {
	_log->log(LOG_DEBUG, RH_NAME,
			  "Parsing Path type.");
	size_t pos = _path.find('?');
	if (pos == std::string::npos) {
		_path_type = PATH_REGULAR;
		_log->log(LOG_DEBUG, RH_NAME,
				  "Regular Path to normalize.");
		return;
	}
	_query_string = _path.substr(pos + 1);
	_path = _path.substr(0, pos);
	_path_type = PATH_QUERY;
	_log->log(LOG_DEBUG, RH_NAME,
			  "Query found and parse from path.");
}

/**
 * @brief Loads and validates header data, including Content-Length, Content-Type, and boundary for multipart data.
 *
 * This method extracts and verifies critical information from the HTTP request header, including
 * `Content-Length` and `Content-Type`. It also checks for the `boundary` parameter if the content is multipart.
 *
 * @details
 * - Retrieves `Content-Length` from `_header`, validates it, and assigns it to `_content_length`.
 * - Retrieves `Content-Type` from `_header`. If it is multipart, it checks for the presence of `boundary`.
 * - If `Content-Length` or `boundary` is malformed or missing when required, it disables sanity and logs the error.
 * - For multipart data, trims `_content_type` to exclude any parameters after the type (e.g., `; boundary=`).
 *
 * @return None
 */
void HttpRequestHandler::load_header_data() {
	_log->log(LOG_DEBUG, RH_NAME, _header);
	std::string content_length = get_header_value(_header, "content-length:", "\r\n");
	_content_type = get_header_value(_header, "content-type:", "\r\n");

	if (!content_length.empty()){
		if (is_valid_size_t(content_length)) {
			_content_length = str_to_size_t(content_length);
		} else {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Content-Length malformed.");
		}
	} else {
		_content_length = 0;
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
	std::string chunks = get_header_value(_header, "transfer-encoding:", "\r\n");
	_log->log(LOG_DEBUG, RSP_NAME, chunks);
	if (!chunks.empty()) {
		chunks = trim(chunks, " ");
		if (chunks == "chunked") {
			_chunks = true;
		}
	}
	_range = get_header_value(_header, "range:", "\r\n");
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
 * @brief Evaluates if the requested path corresponds to a CGI script and adjusts path attributes accordingly.
 *
 * This method checks if the requested path points to a CGI file or a mapped CGI script in the configuration.
 * If a CGI path is detected, it updates `_normalized_path`, `_script`, `_path_info`, and sets `_cgi` to true.
 *
 * @details
 * - If the path is a CGI file directly (not a directory), it sets `_script` and `_normalized_path` appropriately.
 * - Searches for mapped CGI paths in `_location->cgi_locations`. If a match is found, `_normalized_path` and `_script` are updated.
 * - Sets `_cgi` to true if the path corresponds to a CGI file or location.
 * - Logs each step, including the path, type, and any CGI configurations detected.
 *
 * @return None
 */
void HttpRequestHandler::cgi_normalize_path() {
	if (!_location->cgi_file) {
		_log->log(LOG_DEBUG, RH_NAME,
		          "No CGI locations at server config.");
		return ;
	}
	std::string eval_path = _config.server_root + _path;
	if (is_dir(eval_path)) {
		_log->log(LOG_DEBUG, RH_NAME,
		          "CGI test - Directory exist.");
		return ;
	}
	if (eval_path[eval_path.size() - 1] != '/' && is_file(eval_path) && is_cgi(eval_path)) {
		_log->log(LOG_DEBUG, RH_NAME,
		          "The user is over a real CGI file. It should be handle as script.");
		size_t dot_pos = eval_path.find_last_of('/');
		_script = eval_path.substr(dot_pos + 1);
		_normalized_path = eval_path.substr(0, dot_pos);
		_cgi = true;
		return ;
	}

	std::string saved_key;
	const t_cgi* cgi_data = NULL;

	_log->log(LOG_DEBUG, RH_NAME,
	          "Request will be testing against mapped CGI scripts.");
	for (std::map<std::string, t_cgi>::const_iterator it = _location->cgi_locations.begin();
	     it != _location->cgi_locations.end(); it++) {
		const std::string& key = it->first;
		if (starts_with(_path, key)) {
			if (key.length() > saved_key.length()) {
				cgi_data = &it->second;
				saved_key = key;
			}
		}
	}
	if (cgi_data) {
		_log->log(LOG_DEBUG, RH_NAME,
		          "CGI - Location Found: " + saved_key);
		_normalized_path = _config.server_root + cgi_data->cgi_path;
		_script = cgi_data->script;
		_path_info = _path.substr(saved_key.length());
		_cgi = true;
	}
}

/**
 * @brief Normalizes the requested path based on the server root and validates its existence and type.
 *
 * This method combines the server root with the requested `_path`, and then checks if the resulting path
 * is valid according to the HTTP method used (GET, POST, DELETE).
 *
 * @details
 * - If CGI is true, this step won't be executed. CGI paths are normalized previously.
 * - **POST**: Ensures the path is a directory to create a new resource. Sets `_normalized_path` if valid.
 * - **File Check**: For GET requests, checks if the path points to an existing file, updates `_cgi` if it is a CGI, and sets `_status` to `HTTP_OK`.
 * - **DELETE**: Ensures the resource exists; if the target is a directory, the operation is not allowed.
 * - **Directory Check**: If the path is a directory, attempts to locate default files. If found, sets `_normalized_path`.
 * - If no valid path or resource is found, it disables sanity and sets `_status` to `HTTP_NOT_FOUND`.
 *
 * @return None
 */
void HttpRequestHandler::normalize_request_path() {
	if (_cgi) {
		_log->log(LOG_DEBUG, RH_NAME,
		          "CGI context. path has been normalized");
		return ;
	}
	std::string eval_path = _config.server_root + _path;

	_log->log(LOG_DEBUG, RH_NAME,
			  "Normalize path to get proper file to serve." + eval_path);
	if (_method == METHOD_POST) {
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
		_normalized_path = eval_path;
		return ;
	}
	if (eval_path[eval_path.size() - 1] != '/' && is_file(eval_path)) {
		_log->log(LOG_INFO, RH_NAME, "File found.");
		_normalized_path = eval_path;
		_cgi = is_cgi(_normalized_path);
		_status = HTTP_OK;
		return ;
	}
	if (_method == METHOD_DELETE) {
		turn_off_sanity(HTTP_NOT_FOUND,
		                "Resource to be deleted, not found.");
		return ;
	}
	if (is_dir(eval_path)) {
		if (_method == METHOD_DELETE) {
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
				_normalized_path = eval_path + _location->loc_default_pages[i];
				_cgi = is_cgi(_normalized_path);
				_status = HTTP_OK;
				return ;
			}
		}
	}
	turn_off_sanity(HTTP_NOT_FOUND,
	                "Requested path not found " + _path);
}

/**
 * @brief Loads the request content based on the specified transfer encoding (chunks or normal).
 *
 * This method checks whether the request content is chunked by evaluating `_chunks`.
 * It then calls either `load_content_chunks()` or `load_content_normal()` to handle the loading process.
 *
 * @details
 * - Logs the type of content loading (chunked or normal) for tracing purposes.
 * - Calls `load_content_chunks()` if `_chunks` is true, otherwise calls `load_content_normal()`.
 *
 * @return None
 */
void HttpRequestHandler::load_content() {
	if (_chunks) {
		_log->log(LOG_DEBUG, RH_NAME,
				  "Chunk content. Load body request.");
		load_content_chunks();
		_content_length = _body.length();
	} else {
		_log->log(LOG_DEBUG, RH_NAME,
				  "Normal content. Load body request.");
		load_content_normal();
	}
	_log->log(LOG_DEBUG, RH_NAME, _request);
	_log->log(LOG_DEBUG, RH_NAME, _body);
}

/**
 * @brief Loads and processes chunked HTTP content from the request, handling each chunk size and content data.
 *
 * This method loads the request content in chunks by repeatedly calling `parse_chunks()` to extract
 * and validate each chunk based on the Transfer-Encoding. The method manages the total size of
 * the content to ensure it does not exceed `_max_request` and enforces a read timeout.
 *
 * @details
 * - Utilizes `chronos` to prevent indefinite blocking by timing out after a specified period.
 * - Calls `parse_chunks()` to process each chunk and append data to `_request`.
 * - If the request size exceeds `_max_request` or the read fails, it disables sanity.
 *
 * @return None
 */
void HttpRequestHandler::load_content_chunks() {
	char buffer[BUFFER_REQUEST];
	int read_byte;
	size_t size = 0;
	std::string chunk_data = _request;
	_request.clear();
	long chunk_size = 1;

	while (true) {
		if (!_client_data->chronos()) {
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
		turn_off_sanity(HTTP_CLIENT_CLOSE_REQUEST,
		                "Client Close Request");
		return ;
	}
	if (read_byte < 0 && size == 0) {
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
		                "Error Reading From Socket. " + int_to_string(read_byte));
		return ;
	}
	_body = _request;
	_log->log(LOG_DEBUG, RH_NAME,
			  "Chunked Request read.");
}

/**
 * @brief Parses chunked data, validating chunk size and appending content to the request body.
 *
 * This method processes a buffer of chunked HTTP data, where each chunk is prefixed by a hexadecimal
 * size indicator. It validates the format and size of each chunk, appending valid chunks to `_request`.
 *
 * @details
 * - Parses `chunk_data` to find each chunk's size. If the chunk size is zero, it checks for the end of data.
 * - Validates that each chunk does not exceed `_max_request` and that `chunk_size` is properly formatted.
 * - If the chunked data is invalid (e.g., malformed chunk size or missing CRLF), disables sanity and returns `false`.
 *
 * @param[in,out] chunk_data A reference to the string containing raw chunked data.
 * @param[out] chunk_size A reference to store the size of the current chunk for processing.
 * @return `true` if all chunks are parsed and appended successfully, `false` if any error occurs.
 */
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


/**
 * @brief Loads the HTTP request content from the client socket when we are dealing with a "normal" request.
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
void HttpRequestHandler::load_content_normal() {
	if (_content_length == 0) {
		_log->log(LOG_INFO, RH_NAME,
		          "No Content-Length to read from FD.");
		return ;
	}

	char buffer[BUFFER_REQUEST];
	int         read_byte;
	size_t      size = 0;
	size_t      to_read = _content_length - _request.length();

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
 * @brief Creates a request wrapper and delegates processing to HttpResponseHandler.
 *
 * This method initializes an `s_request` structure to encapsulate the details of the current request.
 * It then creates an `HttpResponseHandler` instance, passing the request wrapper and other relevant data,
 * and calls `handle_request()` to manage the response.
 *
 * @details
 * - Constructs `s_request` with all necessary request details, including `_body`, `_method`, `_path`, and CGI indicators.
 * - Delegates the wrapped request to `HttpResponseHandler` for further processing and response handling.
 *
 * @return None
 */
void HttpRequestHandler::handle_request() {
	s_request request_wrapper = s_request(_body, _method, _path,
	                                      _normalized_path, _access, _sanity,
	                                      _status, _content_length, _content_type,
										  _boundary, _path_type, _query_string, _cgi,
	                                      _script, _path_info, _chunks, _all_headers,
										  _range);
	HttpResponseHandler response(_location, _log, _client_data, request_wrapper, _fd);
	response.handle_request();
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
