/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/11 19:29:26 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequestHandler.hpp"

/**
 * @brief Constructs an HttpRequestHandler instance to process and manage HTTP requests.
 *
 * This constructor initializes the handler with essential pointers and settings for processing HTTP requests.
 * It retrieves configuration details from the associated client data, initializes logging and caching resources,
 * and verifies the validity of the essential components.
 *
 * @param log Pointer to a Logger instance for logging request handling activities.
 * @param client_data Pointer to ClientData associated with the current client connection.
 * @param cache Pointer to a WebServerCache instance to manage cached HTTP responses.
 *
 * @throw Logger::NoLoggerPointer if the provided logger pointer is null.
 * @throw WebServerException if any of the provided pointers (client_data or cache) are null.
 *
 * @warning Throws exceptions if essential components (logger, client_data, or cache) are not available,
 * which compromises server functionality.
 */
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
	_factory = 0;
}

/**
 * @brief Destructor for HttpRequestHandler.
 *
 * @note This destructor does not perform specific memory deallocation
 * as most resources are managed externally, but it indicates a cleanup phase in logs.
 */
HttpRequestHandler::~HttpRequestHandler() {
	_log->log(LOG_DEBUG, RH_NAME,
	          "HttpRequestHandler resources clean up.");
}

/**
 * @brief Executes the workflow for processing and validating an HTTP request.
 *
 * This method performs a sequential series of validation and parsing steps
 * to handle the incoming HTTP request. Each step is represented as a function pointer
 * in the `steps` array, allowing dynamic invocation of the individual request processing functions.
 * If a step fails and the request's sanity is set to `false`, the workflow halts.
 *
 * Steps included in the workflow:
 * - Reading the request header
 * - Parsing the header, HTTP method, and path
 * - Determining path type (e.g., regular, query)
 * - Loading header and body content data
 * - Validating request specifics and configurations
 *
 * @note After parsing and validating, this method delegates the response handling
 * to the `handle_request` method, which completes the request-response cycle.
 *
 * @warning If any step invalidates the request (sets sanity to false),
 * further processing is halted, and the request goes to handle request to
 * send the proper error message.
 *
 * @see validate_step
 * @see handle_request
 */
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

/**
 * @brief Reads the HTTP request header from the client socket.
 *
 * This method reads data from the client socket, accumulating it into
 * `_request` until the complete HTTP header is received. The header is considered
 * complete when the "\r\n\r\n" delimiter is detected.
 *
 * Function Workflow:
 * 1. Attempt to read data from the socket using `recv()`.
 * 2. If data is read successfully, append it to the request buffer.
 * 3. Check if the headers are fully received by looking for the end sequence (`\r\n\r\n`).
 * 4. If `recv()` returns `-1`, retry reading up to a predefined maximum number of times.
 * 5. If the client closes the connection (`recv()` returns `0`) or an error occurs after maximum retries,
 * 	  mark the connection as inactive.
 * 6. If the request headers are successfully read, proceed to the next step in handling the request.
 *
 * Upon successful completion, the client's `chronos_reset` is called to refresh
 * the timestamp, indicating the request's active state.
 *
 * @exception std::exception If an unexpected exception occurs during reading,
 * it logs the exception message and disables request sanity.
 *
 * @note This function sets the request's sanity to `false` if any issues are
 * encountered (e.g., timeout, client disconnection, header size too large).
 *
 * @see turn_off_sanity
 * @see ClientData::chronos_request
 */
void HttpRequestHandler::read_request_header() {
	char buffer[BUFFER_REQUEST];
	int read_byte;
	size_t size = 0;
	int retry_count = 0;

	try {
		_log->log(LOG_DEBUG, RH_NAME,
				  "Reading HTTP request");
		while (true) {
			read_byte = recv(_fd, buffer, sizeof(buffer), 0);
			if (read_byte > 0) {
				size += read_byte;
				if (size > _max_request) {
					turn_off_sanity(HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE,
									"Request Header too large.");
					return;
				}
				_request.append(buffer, read_byte);
				if (_request.find("\r\n\r\n") != std::string::npos) {
					break;
				}
				retry_count = 0;
				if (!_client_data->chronos_request()) {
					turn_off_sanity(HTTP_REQUEST_TIMEOUT,
									"Request Timeout.");
					return;
				}
			} else if (read_byte == 0) {
				_client_data->kill_client();
				turn_off_sanity(HTTP_CLIENT_CLOSE_REQUEST,
								"Client Close Request");
				return;
			} else {
				retry_count++;
				if (retry_count >= WS_MAX_RETRIES) {
					turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
									"Max retries exceeded while reading from socket.");
					return;
				}
				usleep(WS_RETRY_DELAY_MICROSECONDS);
				continue;
			}
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

/**
 * @brief Parses the HTTP request header from the `_request` data.
 *
 * This method extracts the HTTP header section from `_request` by locating
 * the header-body delimiter ("\r\n\r\n"). If the delimiter is found,
 * the header data is stored in `_request_data.header` and the remaining
 * data is retained in `_request` as the body. If no delimiter is found,
 * the request is marked as having a bad request error.
 *
 * Key behaviors:
 * - If the header is empty after extraction, the request is marked as invalid
 *   with an HTTP status of `HTTP_BAD_REQUEST`.
 * - If parsing is successful, logs a debug message indicating successful
 *   header parsing.
 * - The body data is preserved for further processing, while `_request`
 *   is cleared to avoid duplication.
 *
 * @note This function sets the request's sanity to `false` if no header-body
 * delimiter is found or if the extracted header is empty.
 *
 * @see turn_off_sanity
 */
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

/**
 * @brief Extracts and validates the HTTP method and path from the request header.
 *
 * This method processes the `_request_data.header` to identify and extract
 * the HTTP method and the request path. If either the method or path is
 * malformed or missing, the request is marked as invalid with an appropriate
 * HTTP status code and error message.
 *
 * Process overview:
 * - The method is extracted by locating the first space in the header.
 * - The path is identified by locating the next space after the method.
 *
 * Key behaviors:
 * - Sets the HTTP method by converting the extracted string to its
 *   corresponding enum type. If the conversion fails, the request
 *   is marked as having a bad request status.
 * - Checks that the extracted path does not exceed `URI_MAX` characters.
 * - Logs the status of parsing and validation, with detailed error
 *   messages if parsing fails.
 *
 * Error handling:
 * - If the method is missing or invalid, the request is marked with
 *   `HTTP_BAD_REQUEST` status.
 * - If the path is missing or exceeds the allowed length, the request
 *   is marked with `HTTP_URI_TOO_LONG` or `HTTP_BAD_REQUEST`.
 *
 * @see turn_off_sanity
 */
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

/**
 * @brief Parses and loads essential data from the HTTP request header.
 *
 * This method extracts critical HTTP header fields such as `Content-Length`,
 * `Content-Type`, `Transfer-Encoding`, `Range`, and `Connection` from the
 * `_request_data.header` string. This information is stored in `_request_data`
 * for further processing.
 *
 * Detailed parsing flow:
 * - **Content-Length**: Ensures the header is a valid size, converting to `size_t`
 *   and storing it in `_request_data.content_length`.
 * - **Content-Type**: Determines the type of content (e.g., multipart) and checks
 *   for a boundary parameter if multipart. It adjusts `_request_data.boundary`
 *   as needed.
 * - **Transfer-Encoding**: Checks for chunked transfer encoding, enabling
 *   `_request_data.chunks` if detected.
 * - **Range**: Loads range data into `_request_data.range` and updates `_factory`
 *   for specific response handling if required.
 * - **Connection**: Sets connection status to keep-alive if specified; otherwise,
 *   marks the connection to be closed.
 * - **Cookie**: Retrieves any cookie data from the header.
 *
 * Error Handling:
 * - If `Content-Length` or `boundary` values are malformed, `sanity` is set to false
 *   with appropriate HTTP error statuses, preventing further request processing.
 *
 * @see get_header_value
 * @see turn_off_sanity
 */
void HttpRequestHandler::load_header_data() {
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

/**
 * @brief Retrieves and sets the location configuration based on the request path.
 *
 * This method searches for the most specific matching location within `_config.locations`
 * based on the `_request_data.path`. The `LocationConfig` object associated with the longest
 * matching key is selected, allowing for precise path matching. Once found, `_location` and
 * `_request_data.access` are updated to reflect this configuration.
 *
 * Detailed Workflow:
 * - Iterates through `_config.locations` to find entries whose paths match the beginning of
 *   `_request_data.path`.
 * - The longest matching key is selected, ensuring the most specific location configuration
 *   is applied.
 * - If a match is found, `_location` is set to the corresponding `LocationConfig` pointer,
 *   and `_request_data.access` is updated.
 * - If no match is found, the method sets `sanity` to false with an HTTP 400 (Bad Request)
 *   status, as the requested path does not correspond to any configured location.
 *
 * Error Handling:
 * - If no matching location is found, `turn_off_sanity` is invoked with `HTTP_BAD_REQUEST`
 *   and a relevant error message, marking the request as invalid.
 *
 * @see turn_off_sanity
 */
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

/**
 * @brief Normalizes the request path for CGI execution if applicable.
 *
 * This method determines if the request path should be handled as a CGI script. It first checks
 * if there are CGI locations specified in the configuration. If CGI is enabled, the method verifies
 * if the requested path corresponds to a CGI script or if it matches any predefined CGI location mappings.
 *
 * Detailed Workflow:
 * - If `_location->cgi_file` is not set, CGI handling is not enabled, and the function exits early.
 * - Constructs `eval_path` by combining `_config.server_root` and `_request_data.path`.
 * - Checks if `eval_path` is a directory or a non-CGI file. If so, it is not treated as a CGI path.
 * - If `eval_path` points to a CGI file directly, it is marked for CGI handling:
 *   - Sets `_request_data.script` to the filename part of the path.
 *   - Sets `_request_data.normalized_path` to the directory containing the CGI file.
 *   - Marks `_request_data.cgi` as `true` and increments `_factory`.
 * - If no direct CGI file is found, checks if the path matches any mapped CGI locations in `_location->cgi_locations`:
 *   - Iterates over `cgi_locations` to find the longest matching prefix.
 *   - If a match is found, sets `_request_data.normalized_path`, `_request_data.script`, and `_request_data.path_info`.
 *   - Marks `_request_data.cgi` as `true` and increments `_factory`.
 *
 * Error Handling:
 * - This method assumes that if a CGI configuration is specified but not found in the request path, it will not
 *   affect the request’s validity. Instead, CGI handling is simply skipped.
 *
 * @see is_file, is_dir, starts_with, _request_data
 */
void HttpRequestHandler::cgi_normalize_path() {
	if (!_location->cgi_file) {
		_log->log(LOG_DEBUG, RH_NAME,
		          "No CGI locations at server config.");
		return ;
	}
	std::string eval_path = _config.server_root + _request_data.path;
	if (is_dir(eval_path) || (is_file(eval_path) && !is_cgi(eval_path))) {
		_log->log(LOG_DEBUG, RH_NAME,
		          "CGI test - Directory or resource exist.");
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

/**
 * @brief Normalizes the request path for file serving and resource validation.
 *
 * This method determines the appropriate file or directory to serve based on `_request_data.path`.
 * It also verifies if the path meets specific requirements for various HTTP methods.
 *
 * Workflow:
 * - **CGI Context**: If `_request_data.cgi` is true, the path is already normalized for CGI, and the function exits early.
 * - **Build and Validate Path**: Constructs `eval_path` by combining `_config.server_root` and `_request_data.path`.
 * - **POST Request**:
 *   - If the HTTP method is POST, ensures `eval_path` is a directory and allows resource creation within it.
 *   - Updates `_request_data.normalized_path` with `eval_path` if it passes validation.
 * - **File Handling**:
 *   - Checks if `eval_path` points to an existing file. If found, it is set as `_request_data.normalized_path`, and CGI handling is verified.
 * - **DELETE Request**:
 *   - If the HTTP method is DELETE, validates that the path is a file and not a directory.
 * - **Directory Handling**:
 *   - For other request types, verifies if `eval_path` points to a directory.
 *   - If a directory is found, attempts to locate a default file within it based on `_location->loc_default_pages`.
 *   - If a valid file is found, updates `_request_data.normalized_path` and confirms the HTTP status.
 *
 * Error Handling:
 * - Sets `sanity` to false if the path does not meet required conditions, such as:
 *   - **POST Method**: The directory does not exist or is invalid.
 *   - **DELETE Method**: The requested resource does not exist or is a directory.
 *   - **File/Directory Not Found**: The requested path or a default file within a directory is not found.
 *
 * @see is_file, is_dir, _request_data, turn_off_sanity
 */
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

/**
 * @brief Loads the request body content based on the transfer encoding.
 *
 * This method handles both chunked and regular content loading based on `_request_data.chunks`.
 *
 * Workflow:
 * - **Chunked Content**: If `_request_data.chunks` is set to `true`, it loads the content using `load_content_chunks()`.
 *   - This approach is typically used when the `Transfer-Encoding` header specifies `chunked`.
 * - **Regular Content**: If `_request_data.chunks` is `false`, it loads the content as a continuous body with `load_content_normal()`.
 *
 * Logging:
 * - Logs the content loading method (chunked or normal) based on the transfer encoding.
 *
 * @see load_content_chunks, load_content_normal
 */
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

/**
 * @brief Loads the HTTP request body content in chunked transfer mode.
 *
 * This function reads the request body content from the client socket when the transfer encoding is chunked.
 * It processes the incoming chunks until the entire request body is received.
 * The function handles errors, request timeouts, and ensures that the body content does not exceed the allowed size.
 *
 * The flow of the function is as follows:
 * 1. Begin by attempting to parse any existing chunk data.
 * 2. Continue reading from the socket using `recv()` to gather more chunk data as needed.
 * 3. If `recv()` returns `-1`, retry reading up to a predefined maximum number of times.
 * 4. If `recv()` returns `0`, it indicates that the client has closed the connection, and the connection is marked inactive.
 * 5. Accumulate the chunk data and parse it to extract individual chunks, appending them to the request body.
 * 6. If the chunk size reaches `0`, it indicates the end of the chunked transfer, and the function completes.
 * 7. If the body is successfully read, reset the request timeout and store the final body content.
 *
 * @see parse_chunks, chronos_request, turn_off_sanity
 */
void HttpRequestHandler::load_content_chunks() {
	char buffer[BUFFER_REQUEST];
	int read_byte;
	size_t size = 0;
	std::string chunk_data = _request;
	_request.clear();
	long chunk_size = 1;
	int retry_count = 0;

	try {
		while (true) {
			if (!_client_data->chronos_request()) {
				turn_off_sanity(HTTP_REQUEST_TIMEOUT,
								"Request Timeout.");
				return;
			}
			parse_chunks(chunk_data, chunk_size);
			if (chunk_size == 0) {
				size = _request.length();
				break;
			}
			read_byte = recv(_fd, buffer, sizeof(buffer), 0);

			if (read_byte > 0) {
				size += read_byte;
				if (size > _max_request) {
					turn_off_sanity(HTTP_CONTENT_TOO_LARGE,
									"Body Content too Large.");
					return;
				}
				chunk_data.append(buffer, read_byte);
				retry_count = 0;

			} else if (read_byte == 0) {
				_client_data->kill_client();
				turn_off_sanity(HTTP_CLIENT_CLOSE_REQUEST,
								"Client Close Request");
				return;

			} else {
				retry_count++;
				if (retry_count >= WS_MAX_RETRIES) {
					turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
									"Max retries exceeded while reading from socket.");
					return;
				}
				usleep(WS_RETRY_DELAY_MICROSECONDS);
				continue;
			}
		}

		if (size == 0) {
			_client_data->kill_client();
			turn_off_sanity(HTTP_CLIENT_CLOSE_REQUEST,
							"Client Close Request");
			return;
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


/**
 * @brief Parses the chunked data according to HTTP/1.1 chunked transfer encoding.
 *
 * This function reads the chunk size and content in the chunked transfer-encoded data,
 * handling each chunk iteratively. It appends valid chunks to `_request` and checks
 * for valid boundaries and chunk sizes.
 *
 * Workflow:
 * - **Chunk Size Extraction**: Reads the chunk size (in hexadecimal) from `chunk_data`.
 * - **Size Validation**: Checks that the chunk size is valid and within specified limits.
 * - **Content Appending**: Appends chunk content to `_request` if within the max size.
 * - **Chunk End Check**: Verifies chunk termination using `\r\n` characters.
 *
 * Sanity Control:
 * - **Invalid Chunk Size**: Calls `turn_off_sanity` with `HTTP_BAD_REQUEST` if the chunk size
 *   format is invalid.
 * - **Excessive Content**: Calls `turn_off_sanity` with `HTTP_CONTENT_TOO_LARGE` if the
 *   cumulative content size exceeds `_max_request`.
 * - **Invalid End of Chunk**: Calls `turn_off_sanity` if the chunk ending is malformed.
 *
 * @param chunk_data The incoming chunked data from the HTTP request body.
 * @param chunk_size A reference to store the size of the current chunk.
 * @return `true` if parsing completes successfully, `false` otherwise.
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
 * @brief Reads the HTTP request body based on the `Content-Length` specified.
 *
 * This function reads the request body from the file descriptor `_fd` until the total
 * number of bytes read matches the `Content-Length`. It updates the `_request` string
 * with the body content and checks for size limits and timeouts.
 *
 * The flow of the function is as follows:
 * 1. Verify that `Content-Length` is greater than `0`. If not, log the information and return.
 * 2. Read data from the socket using `recv()` until the entire content is received.
 * 3. If `recv()` returns `-1`, retry reading up to a predefined maximum number of times.
 * 4. If `recv()` returns `0`, it means the client closed the connection, and the connection is marked inactive.
 * 5. Accumulate the read data in the request buffer and ensure it does not exceed the allowed maximum size.
 * 6. If the body is successfully read, reset the request timeout and proceed with processing the request.
 *
 * Sanity Control:
 * - **Content-Length Exceeded**: Calls `turn_off_sanity` with `HTTP_CONTENT_TOO_LARGE`.
 * - **Timeout Error**: Calls `turn_off_sanity` with `HTTP_REQUEST_TIMEOUT` if reading
 *   times out.
 * - **Socket Closure/Read Error**: Calls `turn_off_sanity` if there is an error reading
 *   from the socket or if the client closes the connection.
 *
 * @exception std::exception Catches general exceptions during the reading process
 * and deactivates sanity if an exception is encountered.
 */
void HttpRequestHandler::load_content_normal() {
	if (_request_data.content_length == 0) {
		_log->log(LOG_INFO, RH_NAME,
				  "No Content-Length to read from FD.");
		return;
	}
	char buffer[BUFFER_REQUEST];
	int read_byte;
	size_t size = 0;
	size_t to_read = _request_data.content_length - _request.length();
	int retry_count = 0;

	try {
		while (to_read > 0) {
			read_byte = recv(_fd, buffer, sizeof(buffer), 0);
			if (read_byte > 0) {
				size += read_byte;
				to_read -= read_byte;

				if (size > _max_request) {
					turn_off_sanity(HTTP_CONTENT_TOO_LARGE,
									"Body Content too Large.");
					return;
				}
				_request.append(buffer, read_byte);
				retry_count = 0;
				if (!_client_data->chronos_request()) {
					turn_off_sanity(HTTP_REQUEST_TIMEOUT,
									"Request Timeout.");
					return;
				}
			} else if (read_byte == 0) {
				_client_data->kill_client();
				turn_off_sanity(HTTP_CLIENT_CLOSE_REQUEST,
								"Client Close Request");
				return;
			} else {
				retry_count++;
				if (retry_count >= WS_MAX_RETRIES) {
					turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
									"Max retries exceeded while reading from socket.");
					return;
				}
				usleep(WS_RETRY_DELAY_MICROSECONDS);
				continue;
			}
		}
		if (size == 0) {
			_client_data->kill_client();
			turn_off_sanity(HTTP_CLIENT_CLOSE_REQUEST,
							"Client Close Request");
			return;
		}
		_client_data->chronos_reset();
		_request_data.body = _request;
		_log->log(LOG_DEBUG, RH_NAME,
				  "Request body read.");
	}
	catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Error reading request: " << e.what();
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
						detail.str());
	}
}

/**
 * @brief Validates the HTTP request based on the request method and body presence.
 *
 * This function checks for logical consistency between the HTTP request method
 * and the presence of a request body. Certain methods (e.g., GET, HEAD, OPTIONS)
 * should not include a body, while others (e.g., POST, PUT, PATCH) require one.
 *
 * - **Methods with Body Restriction**: For methods like GET, HEAD, or OPTIONS,
 *   the presence of a body results in a `HTTP_BAD_REQUEST` status.
 * - **Methods Requiring a Body**: For methods like POST, PUT, or PATCH, the absence
 *   of a body also results in a `HTTP_BAD_REQUEST` status.
 *
 * @details
 * - Calls `turn_off_sanity` to deactivate the request if there is a method-body
 *   inconsistency.
 * - **Sanity Deactivation**: Sets `sanity` to false if a request-method inconsistency
 *   is found, logging a descriptive error message.
 *
 * Workflow:
 * 1. **Check for Inconsistent Body Presence**:
 *   - For **GET, HEAD, OPTIONS**: Ensures no body is included.
 *   - For **POST, PUT, PATCH**: Ensures a body is present.
 * 2. **Sanity Check**: Calls `turn_off_sanity` with `HTTP_BAD_REQUEST` and an
 *    error message if validation fails.
 */
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

/**
 * @brief Processes the HTTP request and initiates the appropriate response handler.
 *
 * Based on the request's state and attributes, this function initializes and
 * dispatches a suitable response handler:
 * - `HttpResponseHandler`: Handles standard HTTP requests.
 * - `HttpCGIHandler`: Handles CGI requests.
 * - `HttpRangeHandler`: Manages ranged requests.
 * - `HttpMultipartHandler`: Handles multipart data uploads.
 *
 * Workflow:
 * 1. **Sanity Check**: If `_request_data.sanity` is false, a generic error response
 *    is sent via `HttpResponseHandler`.
 * 2. **Request Type Decision**:
 *   - If `_factory` is 0, uses `HttpResponseHandler`.
 *   - If `_request_data.cgi` is true, uses `HttpCGIHandler`.
 *   - If `_request_data.range` is non-empty, uses `HttpRangeHandler`.
 *   - If `_request_data.boundary` is non-empty, uses `HttpMultipartHandler`.
 *
 * **Exception Handling**:
 * - Catches `WebServerException`, `Logger::NoLoggerPointer`, and `std::exception`
 *   to log errors and deactivate the client connection.
 *
 * @exception WebServerException Catches errors from WebServer-specific issues.
 * @exception Logger::NoLoggerPointer Catches logging pointer errors.
 * @exception std::exception Catches unexpected exceptions.
 */
void HttpRequestHandler::handle_request() {
	if (!_client_data->is_alive()) {
		return;
	}
	try {
		if (!_request_data.sanity){
			HttpResponseHandler response(_location, _log, _client_data, _request_data, _fd, _cache);
			response.send_error_response();
			return ;
		}
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

/**
 * @brief Disables the request's sanity state and sets an error status.
 *
 * This method is called when an error is encountered that invalidates the
 * current request, effectively marking it as unsound and readying it for an
 * error response. It logs the provided error detail and updates the request's
 * status and sanity state accordingly.
 *
 * @param status The HTTP status code indicating the error type.
 * @param detail A description of the error encountered, which will be logged.
 *
 * @note Once called, the request will be considered invalid (`sanity` is set to false)
 *       and should be handled by an error response in the workflow.
 */
void HttpRequestHandler::turn_off_sanity(e_http_sts status, std::string detail) {
	_log->log(LOG_ERROR, RH_NAME, detail);
	_request_data.sanity = false;
	_request_data.status = status;
}
