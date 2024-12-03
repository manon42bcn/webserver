/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/12/03 22:02:57 by mporras-         ###   ########.fr       */
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
									   ClientData* client_data):
	_config(client_data->get_server()->get_config()),
	_log(log),
	_client_data(client_data),
	_fd(_client_data->get_fd().fd),
	_request_data(client_data->client_request()),
	_cache(&_config.request_cache){

	if (!log) {
		throw Logger::NoLoggerPointer();
	}
	if (!client_data) {
		throw WebServerException("Client Data is not valid. Server health is compromised.");
	}
	if (_request_data.location) {
		_location = _request_data.location;
	}
	if (_request_data.host_config) {
		_host_config = _request_data.host_config;
		_cache = &_request_data.host_config->request_cache;
	}
	if ((_max_request = _client_data->get_server()->get_config().client_max_body_size) == 0) {
		_max_request = 9999999999;
	}
}

/**
 * @brief Destructor for HttpRequestHandler.
 *
 * @note This destructor does not perform specific memory deallocation
 * as most resources are managed externally, but it indicates a cleanup phase in logs.
 */
HttpRequestHandler::~HttpRequestHandler() {
	_log->log_debug( RH_NAME,
	          "HttpRequestHandler resources clean up.");
}

/**
 * @brief Executes the workflow for processing and validating an HTTP request.
 *
 * This method manages the step-by-step workflow for parsing and validating an HTTP request,
 * followed by handling the request if it is ready. The workflow is divided into distinct
 * validation steps, each responsible for a specific part of the HTTP request processing.
 *
 * @details
 * The method operates as follows:
 * 1. **Define Steps**:
 *    - An array of function pointers (`validate_step`) defines the steps for processing:
 *      - `read_request_header`: Reads the HTTP request header from the client.
 *      - `parse_header`: Parses the header for key-value pairs.
 *      - `parse_method_and_path`: Extracts the HTTP method and requested path.
 *      - `parse_path_type`: Determines the type of resource (e.g., file, directory).
 *      - `load_header_data`: Loads additional header data needed for processing.
 *      - `load_host_config`: Maps the request to the correct host configuration.
 *      - `solver_resource`: Resolves the resource requested by the client.
 *      - `load_content`: Reads the content of the request, if any.
 *      - `validate_request`: Performs final validation of the request's integrity.
 *
 * 2. **Check Input State**:
 *    - If the request is not ready and the socket is readable (`POLLIN`), the validation process begins.
 *
 * 3. **Execute Validation Steps**:
 *    - Iterates through the validation steps sequentially:
 *      - Calls each step using a member function pointer.
 *      - Stops the process if the `sanity` flag indicates a validation failure.
 *
 * 4. **Mark Request as Ready**:
 *    - If all steps succeed, the request is marked as ready for further handling.
 *
 * 5. **Handle Ready Requests**:
 *    - If the request is ready and the socket is writable (`POLLOUT`), the request is processed.
 *
 * @throws Exceptions may occur within individual steps if errors are encountered,
 *         such as malformed headers or resource resolution failures.
 */
void HttpRequestHandler::request_workflow() {
	validate_step steps[] = {&HttpRequestHandler::read_request_header,
	                         &HttpRequestHandler::parse_header,
	                         &HttpRequestHandler::parse_method_and_path,
	                         &HttpRequestHandler::parse_path_type,
	                         &HttpRequestHandler::load_header_data,
							 &HttpRequestHandler::load_host_config,
							 &HttpRequestHandler::solver_resource,
	                         &HttpRequestHandler::load_content,
	                         &HttpRequestHandler::validate_request};

	if (!_request_data.request_ready && _client_data->get_state() & POLLIN) {
		_log->log_debug( RH_NAME,
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
		_request_data.request_ready = true;
	}
	if (_request_data.request_ready && _client_data->get_state() & POLLOUT) {
		handle_request();
	}
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

	try {
		_log->log_debug( RH_NAME,
				  "Reading HTTP request");

		int retry_count = 0;
		while (true) {
			read_byte = recv(_fd, buffer, sizeof(buffer), 0);
			if (read_byte > 0) {
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
				_client_data->kill_client();
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
		_log->log_debug( RH_NAME,
				  "Request read.");

	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Error Getting Header Data Request: " << e.what();
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
						detail.str());
		_client_data->kill_client();
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

		_log->log_debug( RH_NAME,
		          "Header successfully parsed.");
		header_end += 4;
		std::string tmp = _request.substr(header_end);
		_request.clear();
		_request = tmp;
		if (_request_data.header.length() > MAX_HEADER) {
			turn_off_sanity(HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE,
							"Request Header too large.");
		}
	} else {
		turn_off_sanity(HTTP_BAD_REQUEST,
		                "Request parsing error: No header-body delimiter found.");
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
void HttpRequestHandler::parse_method_and_path() {
	std::string path;

	_log->log_debug( RH_NAME,
			  "Parsing Request to get path and method.");
	size_t method_end = _request_data.header.find(' ');
	if (method_end != std::string::npos) {
		_request_data.method_str = _request_data.header.substr(0, method_end);
		if (_request_data.method_str.empty()
			|| (_request_data.method = parse_method(_request_data.method_str)) == 0 ) {
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
			_log->log_debug( RH_NAME,
			          "Request header fully parsed.");
			_request_data.path = path;
			_request_data.path_request = path;
			return ;
		} else {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Error parsing request: missing path.");
			return ;
		}
	} else {
		turn_off_sanity(HTTP_BAD_REQUEST,
		                "Error parsing request: method malformed.");
		_request_data.method = 0;
		return ;
	}
}

/**
 * @brief Parses the type of the request path to determine if it contains a query string.
 *
 * This method analyzes the `_request_data.path` to determine whether the path includes a query component.
 * If a query string is found, it is extracted and the path is updated accordingly.
 *
 * @details
 * The method performs the following steps:
 * 1. **Log Start of Parsing**:
 *    - Logs a debug message indicating the start of the path type parsing process.
 *
 * 2. **Find Query Delimiter**:
 *    - Searches for the character `'?'` in `_request_data.path` to determine if there is a query string.
 *    - If `'?'` is not found (`find()` returns `std::string::npos`):
 *      - Sets `_request_data.path_type` to `PATH_REGULAR`, indicating that the path is a regular path without any query.
 *      - Logs a debug message stating that it is a regular path.
 *
 * 3. **Extract Query String**:
 *    - If `'?'` is found, extracts the query string by taking the part of the path after the `'?'`.
 *    - Updates `_request_data.query` with the extracted query string.
 *    - Updates `_request_data.path` to only contain the part before the `'?'`.
 *    - Sets `_request_data.path_type` to `PATH_QUERY`, indicating that the path includes a query.
 *    - Logs a debug message indicating that a query has been found and parsed.
 *
 * @note
 * - The method modifies `_request_data.path` and `_request_data.query` to separate the base path from the query string.
 * - The `_request_data.path_type` is updated based on whether the path contains a query string or not.
 *
 * **Path Types**:
 * - `PATH_REGULAR`: Indicates a standard path with no query string.
 * - `PATH_QUERY`: Indicates a path that contains a query string.
 */
void HttpRequestHandler::parse_path_type() {
	_log->log_debug( RH_NAME,
			  "Parsing Path type.");
	size_t pos = _request_data.path.find('?');
	if (pos == std::string::npos) {
		_request_data.path_type = PATH_REGULAR;
		_log->log_debug( RH_NAME,
				  "Regular Path to normalize.");
		return;
	}
	_request_data.query = _request_data.path.substr(pos + 1);
	_request_data.path = _request_data.path.substr(0, pos);
	_request_data.path_type = PATH_QUERY;
	_log->log_debug( RH_NAME,
			  "Query found and parse from path.");
}

/**
 * @brief Loads and processes the header data from the HTTP request.
 *
 * This method extracts key information from the request headers and stores them in `_request_data`.
 * It handles headers such as `Content-Length`, `Content-Type`, `Transfer-Encoding`, `Range`, `Connection`, `Cookie`, and `Referer`, and performs necessary checks to ensure the validity of the data.
 *
 * @details
 * The method performs the following actions:
 *
 * 1. **Content-Length Header**:
 *    - Retrieves the `Content-Length` header value using `get_header_value()`.
 *    - If `Content-Length` is present and valid (`is_valid_size_t()` returns `true`), converts it to a `size_t` using `str_to_size_t()` and stores it in `_request_data.content_length`.
 *    - If the `Content-Length` is invalid, sets the request status to `HTTP_BAD_REQUEST`.
 *    - If the `Content-Length` header is not present, sets `_request_data.content_length` to `0`.
 *
 * 2. **Content-Type Header**:
 *    - Retrieves the `Content-Type` header value.
 *    - If the `Content-Type` indicates `multipart`, extracts the `boundary` value.
 *    - If the `boundary` is not found or malformed, sets the request status to `HTTP_BAD_REQUEST`.
 *    - Updates `_request_data.content_type` to only include the primary MIME type, removing any additional parameters.
 *
 * 3. **Transfer-Encoding Header**:
 *    - Retrieves the `Transfer-Encoding` header value.
 *    - Checks if the encoding is `"chunked"` and sets `_request_data.chunks` to `true` if it is.
 *
 * 4. **Range Header**:
 *    - Retrieves the `Range` header value.
 *    - If the `Range` header is present, increments `_request_data.factory` to indicate that this data will be used later.
 *
 * 5. **Connection Header**:
 *    - Retrieves the `Connection` header value.
 *    - If the value is `"keep-alive"`, calls `_client_data->keep_active()` to maintain the connection.
 *    - Otherwise, calls `_client_data->deactivate()` to close the connection after handling the request.
 *
 * 6. **Cookie Header**:
 *    - Retrieves the `Cookie` header value and stores it in `_request_data.cookie`.
 *
 * 7. **Referer Header**:
 *    - Retrieves the `Referer` header value and stores it in `_request_data.referer`.
 *
 * 8. **Host**:
 * 	  - Retrieves `Host` header value, and stores it in `_request_data.host` to be parsed.
 *
 * @note
 * - The method uses `get_header_value()` to extract specific header values from `_request_data.header`.
 * - The `_request_data.factory` variable is incremented whenever additional processing for multipart content or range is required.
 * - The method ensures that malformed headers, such as an invalid `Content-Length` or missing `boundary`, are caught and the request is marked as invalid.
 */
void HttpRequestHandler::load_header_data() {
	std::string content_length = get_header_value(_request_data.header, "content-length:");
	_request_data.content_type = get_header_value(_request_data.header, "content-type:");

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
			_request_data.boundary = get_header_value(_request_data.content_type, "boundary");
			if (_request_data.boundary.empty()) {
				turn_off_sanity(HTTP_BAD_REQUEST,
				                "Boundary malformed or not present with a multipart Content-Type.");
			}
			_request_data.factory++;
			size_t end_type = _request_data.content_type.find(';');
			if (end_type != std::string::npos) {
				_request_data.content_type = _request_data.content_type.substr(0, end_type);
			}
		}
	}
	_request_data.host = get_header_value(_request_data.header, "host:");
	std::string chunks = get_header_value(_request_data.header, "transfer-encoding:");
	if (!chunks.empty()) {
		chunks = trim(chunks, " ");
		if (chunks == "chunked") {
			_request_data.chunks = true;
		}
	}
	_request_data.range = get_header_value(_request_data.header, "range:");
	if (!_request_data.range.empty()) {
		_request_data.factory++;
	}
	std::string keep = get_header_value(_request_data.header, "connection:");
	if (keep == "keep-alive") {
		_client_data->keep_active();
	} else {
		_client_data->deactivate();
	}
	_request_data.cookie = get_header_value(_request_data.header, "cookie");
	_request_data.referer = get_header_value(_request_data.header, "referer:");
}

void HttpRequestHandler::load_host_config() {
	std::string host = to_lowercase(_request_data.host);
	_host_config = _client_data->get_server()->get_config(host);
	_request_data.host_config = _host_config;
	_cache = &_request_data.host_config->request_cache;
	if (_host_config == NULL) {
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
						"Host Config Pointer NULL.");
		return ;
	}
	_request_data.host = normalize_host(_request_data.host);
}

/**
 * @brief Resolves the requested resource by following a series of validation steps.
 *
 * This method is responsible for validating and normalizing the request path and CGI configurations to determine the correct location for the requested resource.
 * It executes a sequence of validation steps and, if necessary, attempts to resolve relative paths before repeating the validation.
 *
 * @details
 * The method performs the following steps:
 * 1. **Initialize Validation Steps**:
 *    - Defines an array of function pointers (`steps`) representing the validation steps to be executed in sequence:
 *      - `get_location_config()`: Retrieves the configuration for the requested location.
 *      - `cgi_normalize_path()`: Normalizes the path for CGI requests.
 *      - `normalize_request_path()`: Normalizes the request path.
 *
 * 2. **Execute Initial Validation Steps**:
 *    - Iterates over the `steps` array and executes each function pointer (`(this->*steps[i])()`).
 *    - If any of the steps fail (i.e., `_request_data.sanity` is set to `false`), the loop is terminated.
 *    - Tracks the number of successful steps using `_solver_count`.
 *
 * 3. **Check for Successful Validation**:
 *    - If all three validation steps are successful (`_solver_count == 3`) or if the `Referer` is empty, the method returns.
 *
 * 4. **Reset and Resolve Relative Path**:
 *    - If the validation was not fully successful, resets the `_location` to `NULL` and reinitializes `_request_data` fields:
 *      - `_request_data.sanity` is set to `true`.
 *      - `_request_data.status` is reset to `HTTP_MAX_STATUS`.
 *      - `_request_data.path` is reset to the original request path (`_request_data.path_request`).
 *    - Calls `resolve_relative_path()` to resolve any relative paths in the request.
 *
 * 5. **Execute Validation Steps Again**:
 *    - Repeats the validation steps to ensure that the request path and configurations are now valid after resolving the relative path.
 *    - If any step fails (`_request_data.sanity == false`), the loop is terminated.
 *
 * **Validation Steps**:
 * - `get_location_config()`: Retrieves location-specific settings.
 * - `cgi_normalize_path()`: Ensures the CGI path is properly normalized.
 * - `normalize_request_path()`: Normalizes the request path to ensure consistency.
 */
void HttpRequestHandler::solver_resource() {
	validate_step steps[] = {&HttpRequestHandler::get_location_config,
	                         &HttpRequestHandler::cgi_normalize_path,
	                         &HttpRequestHandler::normalize_request_path};
	size_t i = 0;
	while (i < (sizeof(steps) / sizeof(validate_step)))
	{
		(this->*steps[i])();
		if (!_request_data.sanity)
			break;
		i++;
	}
	if (i == 3 || _request_data.referer.empty()) {
		return ;
	}
	_location = NULL;
	_request_data.location = NULL;
	_request_data.sanity = true;
	_request_data.status = HTTP_MAX_STATUS;
	_request_data.path = _request_data.path_request;
	resolve_relative_path();
	i = 0;
	while (i < (sizeof(steps) / sizeof(validate_step)))
	{
		(this->*steps[i])();
		if (!_request_data.sanity)
			break;
		i++;
	}
}

/**
 * @brief Resolves the relative path of the requested resource based on the `Referer` header.
 *
 * This method processes the `_request_data.referer` field to determine the appropriate base path for resolving the requested resource.
 * It handles different relative path formats (`/`, `./`, `../`) to ensure that the complete path to the resource is correctly constructed.
 *
 * @details
 * The method performs the following steps:
 * 1. **Empty Referer Check**:
 *    - If the `Referer` field is empty, the method returns immediately as there is no context to resolve the path.
 *
 * 2. **Extracting Base Path from Referer**:
 *    - Looks for `"//"` in the `Referer` to identify the beginning of the base path after the protocol (e.g., `http://`).
 *    - If `"//"` is not found, it sets the request as `HTTP_BAD_REQUEST` because the `Referer` field is malformed.
 *    - Extracts the base path starting after the `"//"` to handle the host and URI path.
 *
 * 3. **Handling Root Path (`/`)**:
 *    - Extracts the part of the `Referer` after the first `/` to obtain the base path relative to the server.
 *    - If the requested path starts with `/`, concatenates the base path with the requested path.
 *
 * 4. **Handling Relative Path (`./`)**:
 *    - If the requested path starts with `"./"`, strips the `./` and concatenates the remaining part with the base path.
 *
 * 5. **Handling Parent Directory Path (`../`)**:
 *    - If the requested path contains `"../"`, the method iteratively moves up one level in the base path for each occurrence of `"../"`.
 *    - If the base path cannot move up further (`/` not found), it sets the request status as `HTTP_NOT_FOUND` because the relative path is impossible to reach.
 *    - Finally, appends the remaining part of the path to the updated base path.
 *
 * @note
 * - This method updates `_request_data.path` to contain the fully resolved path based on the relative path specified.
 * - The method assumes that `_request_data.referer` is a valid URL and performs sanity checks to prevent malformed paths.
 *
 * @exception The method uses `turn_off_sanity()` to handle malformed `Referer` or impossible paths, setting appropriate HTTP response codes
 */
void HttpRequestHandler::resolve_relative_path() {
	if (_request_data.referer.empty()) {
		return;
	}
	std::string base = _request_data.referer;
	size_t http_slash = base.find("//");
	if (http_slash != std::string::npos) {
		base = base.substr(http_slash + 2);
	}
	base = normalize_host(base);
	http_slash = base.find('/');
	if (http_slash == std::string::npos) {
		return ;
	}
	base = base.substr(http_slash);

	if (valid_mime_type(base)) {
		size_t base_test = base.find_last_of('/');
		if (base_test != std::string::npos) {
			base = base.substr(0, base_test);
		}
	}
	if (base == "/" || starts_with(_request_data.path, base)) {
		return;
	}
	if (starts_with(_request_data.path, "/")) {
		_request_data.path = base + _request_data.path;
	}
}

/**
 * @brief Retrieves and sets the location configuration based on the request path.
 *
 * This method searches for the most specific matching location within `_host_config->locations`
 * based on the `_request_data.path`. The `LocationConfig` object associated with the longest
 * matching key is selected, allowing for precise path matching.
 *
 * Detailed Workflow:
 * - Iterates through `_host_config->locations` to find entries whose paths match the beginning of
 *   `_request_data.path`.
 * - The longest matching key is selected, ensuring the most specific location configuration
 *   is applied.
 * - If a match is found, `_location` is set to the corresponding `LocationConfig` pointer.
 * - If no match is found, the method sets `sanity` to false with an HTTP 400 (Bad Request)
 *   status, as the requested path does not correspond to any configured location.
 * @note: This method will use cached data if the method is GET and its content is available.
 * 		  cached avoid extra interactions over locations.
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

	_log->log_debug( RH_NAME,
			  "Searching related location.");

	_request_data.is_cached = _cache->get(_request_data.path, _cache_data);
	if (_request_data.is_cached && HAS_GET(_request_data.method)) {
		_location = _cache_data.location;
		_request_data.location = _cache_data.location;
		_request_data.normalized_path = _cache_data.normalized_path;
		return ;
	}
	for (std::map<std::string, LocationConfig>::const_iterator it = _host_config->locations.begin();
	     it != _host_config->locations.end(); ++it) {
		const std::string& key = it->first;
		if (starts_with(_request_data.path, key)) {
			if (key.length() > saved_key.length()) {
				result = &it->second;
				saved_key = key;
			}
		}
	}
	if (result) {
		_log->log_debug( RH_NAME,
				  "Location Found: " + saved_key);
		_location = result;
		_request_data.location = result;
		if (_location->is_redir) {
			_request_data.is_redir = true;
			_request_data.factory++;
		}
		if (!_location->is_root) {
			if (saved_key == "/") {
				_request_data.path = _location->path_root + _request_data.path;
			} else {
				_request_data.path.replace(0, saved_key.length(), _location->path_root);
			}
		}
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
 * - Constructs `eval_path` by combining `_host_config->server_root` and `_request_data.path`.
 * - Checks if `eval_path` is a directory or a non-CGI file. If so, it is not treated as a CGI path.
 * - If `eval_path` points to a CGI file directly, it is marked for CGI handling:
 *   - Sets `_request_data.script` to the filename part of the path.
 *   - Sets `_request_data.normalized_path` to the directory containing the CGI file.
 *   - Marks `_request_data.cgi` as `true` and increments `_request_data.factory`.
 * - If no direct CGI file is found, checks if the path matches any mapped CGI locations in `_location->cgi_locations`:
 *   - Iterates over `cgi_locations` to find the longest matching prefix.
 *   - If a match is found, sets `_request_data.normalized_path`, `_request_data.script`, and `_request_data.path_info`.
 *   - Marks `_request_data.cgi` as `true` and increments `_request_data.factory`.
 *
 * Error Handling:
 * - This method assumes that if a CGI configuration is specified but not found in the request path, it will not
 *   affect the request’s validity. Instead, CGI handling is simply skipped.
 *
 * @see is_file, is_dir, starts_with, _request_data
 */
void HttpRequestHandler::cgi_normalize_path() {
	if (!_location->cgi_file || _request_data.is_cached) {
		_log->log_debug( RH_NAME,
		          "No CGI locations at server config.");
		return ;
	}
	std::string eval_path = _host_config->server_root + _request_data.path;
	if (is_dir(eval_path) || (is_file(eval_path) && !is_cgi(eval_path))) {
		_log->log_debug( RH_NAME,
		          "CGI test - Directory or resource exist.");
		return ;
	}

	if (eval_path[eval_path.size() - 1] != '/' && is_file(eval_path) && is_cgi(eval_path)) {
		_log->log_debug( RH_NAME,
		          "The user is over a real CGI file. It should be handle as script.");
		size_t dot_pos = eval_path.find_last_of('/');
		_request_data.script = eval_path.substr(dot_pos + 1);
		_request_data.normalized_path = eval_path.substr(0, dot_pos);
		_request_data.cgi = true;
		_request_data.factory++;
		return ;
	}

	std::string saved_key;
	const t_cgi* cgi_data = NULL;

	_log->log_debug( RH_NAME,
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
		_log->log_debug( RH_NAME,
		          "CGI - Location Found: " + saved_key);
		_request_data.normalized_path = _host_config->server_root + cgi_data->cgi_path;
		_request_data.script = cgi_data->script;
		_request_data.path_info = _request_data.path.substr(saved_key.length());
		_request_data.cgi = true;
		_request_data.factory++;
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
 * - **Build and Validate Path**: Constructs `eval_path` by combining `_host_config->server_root` and `_request_data.path`.
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
		_log->log_debug( RH_NAME,
		          "CGI context. path has been normalized");
		return ;
	}
	if (_request_data.is_cached || _request_data.is_redir) {
		return;
	}
	std::string eval_path = _host_config->server_root + _request_data.path;

	_log->log_debug( RH_NAME,
			  "Normalize path to get proper file to serve.");
	if (HAS_PERMISSION(_request_data.method, MASK_METHOD_POST)) {
		_log->log_debug( RH_NAME,
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
		_log->log_info( RH_NAME, "File found.");
		_request_data.normalized_path = eval_path;
		_request_data.cgi = is_cgi(_request_data.normalized_path);
		return ;
	}
	if (HAS_PERMISSION(_request_data.method, MASK_METHOD_DELETE)) {
		turn_off_sanity(HTTP_NOT_FOUND,
		                "Resource to be deleted, not found.");
		return ;
	}
	if (is_dir(eval_path)) {
		if (HAS_PERMISSION(_request_data.method, MASK_METHOD_DELETE)) {
			turn_off_sanity(HTTP_METHOD_NOT_ALLOWED,
							"Delete method over a dir is not allowed.");
			return ;
		}
		if (eval_path[eval_path.size() - 1] != '/') {
			eval_path += "/";
		}
		for (size_t i = 0; i < _location->loc_default_pages.size(); i++) {
			_log->log_info( RH_NAME, "here" + eval_path + _location->loc_default_pages[i]);
			if (is_file(eval_path + _location->loc_default_pages[i])) {
				_log->log_info( RH_NAME, "Default File found");
				_request_data.normalized_path = eval_path + _location->loc_default_pages[i];
				_request_data.cgi = is_cgi(_request_data.normalized_path);
				return ;
			}
		}
		if (_location->autoindex && HAS_GET(_request_data.method)) {
			_request_data.factory++;
			_request_data.autoindex = true;
			_request_data.normalized_path = eval_path;
			return ;
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
		_log->log_debug( RH_NAME,
				  "Chunk content. Load body request.");
		load_content_chunks();
	} else {
		_log->log_debug( RH_NAME,
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
				_client_data->kill_client();
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
			_client_data->kill_client();
			return;
		}

		_request_data.body = _request;
		_request_data.content_length = _request_data.body.length();
		_log->log_debug( RH_NAME,
				  "Chunked Request read.");

	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Error getting chunk data: " << e.what();
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
						detail.str());
		_client_data->kill_client();
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
		_log->log_info( RH_NAME,
				  "No Content-Length to read from FD.");
		return;
	}
	char buffer[BUFFER_REQUEST];
	int read_byte;
	size_t size = _request.length();
	if (size > _max_request) {
		turn_off_sanity(HTTP_CONTENT_TOO_LARGE,
						"Body Content too Large.");
		return;
	}
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
				_client_data->kill_client();
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
		_request_data.body = _request;
		_log->log_debug( RH_NAME,
				  "Request body read.");
	}
	catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Error reading request: " << e.what();
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
						detail.str());
		_client_data->kill_client();
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
 * 3. **Security Issues**: If a cgi script is trying to be executed, without cgi
 * 	  active, turn off sanity to avoid server the script as plain text.
 */
void HttpRequestHandler::validate_request() {
	if (is_cgi(_request_data.normalized_path)) {
		if (!_location->cgi_file) {
			turn_off_sanity(HTTP_FORBIDDEN,
			                "Try to execute script without cgi active.");
			return;
		}
	}

	if (!HAS_PERMISSION(_location->loc_allowed_methods, _request_data.method)) {
		turn_off_sanity(HTTP_METHOD_NOT_ALLOWED,
						"Method not allowed at location.");
		return ;
	}
	if (!_request_data.body.empty()) {
		if (HAS_PERMISSION(_request_data.method, MASK_METHOD_GET | MASK_METHOD_HEAD | MASK_METHOD_OPTIONS)) {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Body received with GET, HEAD or OPTION method.");
			return ;
		}
	} else {
		if (HAS_PERMISSION(_request_data.method, MASK_METHOD_POST | MASK_METHOD_PUT | MASK_METHOD_PATCH)) {
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
 *   - If `_request_data.factory` is 0, uses `HttpResponseHandler`.
 *   - If `_request_data.cgi` is true, uses `HttpCGIHandler`.
 *   - If `_request_data.range` is non-empty, uses `HttpRangeHandler`.
 *   - If `_request_data.boundary` is non-empty, uses `HttpMultipartHandler`.
 * @note: This method will cache request related info if it was not cached yet, if
 * 		  everything went ok, and if method is get.
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
			HttpResponseHandler response(_location, _log, _client_data, _request_data, _fd);
			response.send_error_response();
			return ;
		}
		if (_request_data.factory == 0) {
			HttpResponseHandler response(_location, _log, _client_data, _request_data, _fd);
			response.handle_request();
			if (_request_data.is_cached) {
				if (!_request_data.sanity) {
					_cache->remove(_request_data.path);
					return ;
				}
				return;
			}
			if (_request_data.sanity && HAS_GET(_request_data.method)) {
				_cache->put(_request_data.path,
							   CacheRequest(_request_data.path, _host_config,
											_location, _request_data.normalized_path));
			}
			return;
		}
		if (_request_data.is_redir) {
			HttpResponseHandler response(_location, _log, _client_data, _request_data, _fd);
			response.redirection();
			return;
		}
		if (_request_data.autoindex) {
			HttpAutoIndex response(_location, _log, _client_data, _request_data, _fd);
			response.handle_request();
			return;
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
		_log->log_error( RH_NAME,
				  detail.str());
		_client_data->kill_client();
	} catch (Logger::NoLoggerPointer& e) {
		std::ostringstream detail;
		detail << "Logger Pointer Error at Response Handler. "
			   << "Server Sanity could be compromise.";
		_log->log_error( RH_NAME,
		          detail.str());
		_client_data->kill_client();
	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Unknown error handling response: " << e.what();
		_log->log_error( RH_NAME,
		          detail.str());
		_client_data->kill_client();
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
	_log->log_error( RH_NAME, detail);
	_request_data.sanity = false;
	_request_data.status = status;
}
