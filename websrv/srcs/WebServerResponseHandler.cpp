/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServerResponseHandler.cpp                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/07 09:37:41 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/19 23:07:52 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#include "WebServerResponseHandler.hpp"

/**
 * @brief Constructs a `WsResponseHandler` to manage web server responses.
 *
 * Initializes the `WsResponseHandler` with essential components for handling
 * server responses, including location configuration, logging, client data,
 * request details, and the client's file descriptor.
 *
 * @param location Pointer to the `LocationConfig` containing the location-specific configuration.
 * @param log Pointer to the `Logger` instance for logging activities and errors.
 * @param client_data Pointer to the `ClientData` object representing the client connection.
 * @param request Reference to the `s_request` structure with details about the client's request.
 * @param fd File descriptor associated with the client connection.
 *
 * @throws Logger::NoLoggerPointer if the `log` pointer is `NULL`, indicating that logging is required.
 */
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
	if (_log == NULL) {
		throw Logger::NoLoggerPointer();
	}
	if (!client_data) {
		throw WebServerException("Ws Response Handler missing client pointer.");
	}
}

/**
 * @brief Destructor for the `WsResponseHandler` class.
 *
 * Cleans up any resources held by the `WsResponseHandler` instance.
 * Abstract class does not need to free any resource.
 */
WsResponseHandler::~WsResponseHandler(){}

/**
 @section HANDLERS:
 *	handle_request() - Entrypoint -
 *	handle_get()
 *	handle_post()
 *	handle_delete()
 */

/**
 * @brief Handles the incoming HTTP request based on the request method.
 *
 * This method processes the request based on the HTTP method specified in the request,
 * and delegates the actual processing to the appropriate method (`handle_get`, `handle_post`,
 * or `handle_delete`). If the request method is not recognized, it returns a `405 Method Not Allowed`
 * error. Additionally, if the request's sanity check fails, it will trigger an error response.
 *
 * @returns `true` if the request is successfully handled, otherwise `false` if an error occurs.
 * @throws WebServerException if the `_location` pointer is `NULL`, required to handle normal request.
 */
bool WsResponseHandler::handle_request() {

	if (!_location) {
		throw WebServerException("Location pointer not present to handle normal request.");
	}
	switch (_request.method) {
		case MASK_METHOD_GET:
			_log->log_debug( RSP_NAME,
					  "Handle GET request.");
			return (handle_get());
		case MASK_METHOD_POST:
			_log->log_debug( RSP_NAME,
					  "Handle POST request.");
			return 	(handle_post());
		case MASK_METHOD_DELETE:
			_log->log_debug( RSP_NAME,
					  "Handle DELETE request.");
			return (handle_delete());
		default:
			turn_off_sanity(HTTP_NOT_IMPLEMENTED,
							"Method not allowed.");
			return (send_error_response());
	}
}

/**
 * @brief Handles HTTP GET requests, validating access, resource data, and sending a response.
 *
 * This method processes GET requests by checking if the client has read access to the location.
 * If access is allowed, it get file content
 * Upon successful validation, _response_data.content is sent to the client.
 * If any step fails, an error response is sent to the client.
 *
 * @returns `true` if the GET request is successfully handled and the data is properly read;
 *          otherwise, `false` if an error occurs or access is denied.
 */
bool WsResponseHandler::handle_get() {
	if (!HAS_GET(_location->loc_allowed_methods)) {
		turn_off_sanity(HTTP_FORBIDDEN,
						"Get not allowed over resource.");
		send_error_response();
		return (false);
	}
	get_file_content(_request.normalized_path);
	if (_response_data.status) {
		_request.status = HTTP_OK;
		_log->log_debug( RSP_NAME,
				  "File content will be sent.");
		return (send_response(_response_data.content, _request.normalized_path));
	}
	_log->log_debug( RSP_NAME,
			  "Get will send a error due to content load fails.");
	return (send_error_response());
}

/**
 * @brief Handles HTTP POST requests, validating access, saving data, and sending a response.
 *
 * This method processes POST requests by checking if the client has write access to the location.
 * If access is allowed, it validates the payload and resets the client timeout.
 * Upon successful validation, the request body is saved to the specified path, and a success response is sent.
 * If any step fails, an error response is sent to the client.
 *
 * @returns `true` if the POST request is successfully handled and the data is saved;
 *          otherwise, `false` if an error occurs or access is denied.
 */
bool WsResponseHandler::handle_post() {
	if (!HAS_POST(_location->loc_allowed_methods)) {
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

/**
 * @brief Handles HTTP DELETE requests by checking permissions and deleting the specified resource.
 *
 * This method processes DELETE requests by verifying that the client has sufficient permissions.
 * If permissions are adequate, it checks the existence of the resource, deletes it,
 * and sends a success response. If any step fails, an error response is sent to the client.
 *
 * @returns `true` if the resource is successfully deleted and a success response is sent;
 *          `false` if an error occurs, such as insufficient permissions, resource not found,
 *          or failure to delete the resource.
 */
bool WsResponseHandler::handle_delete() {
	if (!HAS_DELETE(_location->loc_allowed_methods)) {
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
	_log->log_debug( RSP_NAME,
	          "Resource deleted successfully: " + delete_path);
	send_response("Resource Deleted.", delete_path);
	return (true);
}

/**
 * @brief Validates the payload of an HTTP request to ensure it meets requirements.
 *
 * This method checks if the request payload satisfies necessary criteria for processing,
 * specifically for POST requests. It verifies that the `Content-Length` and `Content-Type`
 * headers are present, that the request body exists and matches the expected length,
 * and that the file extension is not blacklisted. If any validation fails, the request
 * is marked as invalid and an error response is prepared.
 *
 * @returns `true` if the payload is valid and ready for further processing; `false` otherwise.
 */
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

/**
 @section I/O Methods: read - save file content
 */

/**
 * @brief Retrieves the content of a file specified by the path.
 *
 * This method attempts to read the content of a file located at the given path.
 * If the path is valid and the file can be read, the content is stored in the
 * response data. If any error occurs (e.g., file not found, read error, or an
 * exception), the method logs the error, updates the HTTP status code, and marks
 * the response data as invalid.
 *
 * @param path Path to the file whose content is to be retrieved.
 */
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
			return ;
		}
		file.seekg(0, std::ios::end);
		std::streampos file_size = file.tellg();
		if (file_size == 0) {
			_response_data.status = true;
			return ;
		}
		file.seekg(0, std::ios::beg);
		content.resize(file_size);

		file.read(&content[0], file_size);

		if (!file) {
			turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
			                "Error reading file: " + path);
			return ;
		}
		file.close();
	} catch (const std::ios_base::failure& e) {
		std::ostringstream detail;
		detail << "I/O filure: "<< e.what();
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
						detail.str());
		return ;
	} catch (const std::exception& e) {
		std::ostringstream detail;
		detail << "Unexpected exception: "<< e.what();
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
		                detail.str());
		return ;
	}
	_log->log_debug( RSP_NAME,
			  "File content read OK.");
	_response_data.content = content;
	_response_data.status = true;
}

/**
 * @brief Saves the provided content to a specified file path.
 *
 * This method attempts to save the given content to a file at the specified path.
 * If the file already exists, an error response is set, as overwriting is not permitted.
 * If the file does not exist, it is created, and the content is written to it.
 * Any errors encountered during file operations will log an error, update the HTTP status,
 * and mark the request as failed.
 *
 * @param save_path The path where the content should be saved.
 * @param content The content to be written to the file.
 * @returns `true` if the file is saved successfully; `false` if an error occurs.
 */
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

/**
 @section Response Builders
 */

/**
 * @brief Constructs an HTTP response header based on provided parameters.
 *
 * This method builds an HTTP response header string with status code, content length,
 * content type, connection type, and range support, if applicable. The method supports
 * connection keep-alive if the client is active and the request is valid (`sanity` is true).
 * For ranged responses, the `Content-Range` and `Accept-Ranges` headers are included.
 *
 * @param code HTTP status code for the response.
 * @param content_size Size of the response content in bytes.
 * @param mime MIME type of the response content.
 * @returns A fully constructed HTTP response header as a `std::string`.
 */
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
	return (header.str());
}

/**
 * @brief Sends an HTTP response with the specified body content and inferred MIME type.
 *
 * This method constructs and sends an HTTP response header and body to the client.
 * The MIME type is determined based on the response data, or inferred from the file path
 * if no MIME type is specified. The response header is created using the status code and
 * content length, after which the full response is sent.
 *
 * @param body The body content of the response.
 * @param path The file path used to infer the MIME type if not specified in the response data.
 * @returns `true` if the response is successfully sent; `false` if sending fails.
 */
bool WsResponseHandler::send_response(const std::string &body, const std::string &path) {
	std::string mime_type;
	if (_response_data.mime.empty()) {
		mime_type = get_mime_type(path);
	} else {
		mime_type = _response_data.mime;
	}
	_headers = header(_request.status,
					  _response_data.content.length(), mime_type);
	return(sender(body));
}

/**
 * @brief Sends the HTTP response to the client through the socket file descriptor.
 *
 * This function sends the complete response, including headers and body, to the client.
 * It handles partial writes and ensures that all data is sent correctly by using a loop.
 *
 * The flow of the function is as follows:
 * 1. Concatenate the response headers and body into a single response string.
 * 2. Attempt to send the response data using `send()`.
 * 3. If `send()` returns `-1`, retry sending up to a predefined maximum number of times without checking `errno`.
 * 4. Introduce a small delay between retries to avoid a busy-wait loop.
 * 5. Accumulate the number of bytes successfully sent and continue until the entire response is sent.
 * 6. If the response is sent successfully, log the details and return true. If it fails after maximum retries, return false.
 *
 * @param body The body content of the HTTP response to be sent.
 * @return True if the response is sent successfully, false otherwise.
 */
bool WsResponseHandler::sender(const std::string& body) {
	std::string response = _headers + body;
	int total_sent = 0;
	int to_send = static_cast<int>(response.length());
	int retry_count = 0;
	try {
		while (total_sent < to_send) {
			int sent_bytes = static_cast<int>(send(_fd, response.c_str() + total_sent, to_send - total_sent, 0));
			if (sent_bytes == -1) {
				retry_count++;
				if (retry_count >= WS_MAX_RETRIES) {
					_log->log_error( RSP_NAME,
							  "Max retries exceeded while sending response.");
					return false;
				}
				_log->log_debug( RSP_NAME,
						  "Send failed, retrying...");
				usleep(WS_RETRY_DELAY_MICROSECONDS);
				continue;
			}
			retry_count = 0;
			total_sent += sent_bytes;
		}
		std::ostringstream detail;
		detail << "Response was sent. Status: " << _request.status << " Sent: " << response.length();
		_log->log_debug( RSP_NAME, detail.str());
		return (true);
	} catch (const std::exception& e) {
		_log->log_error( RSP_NAME, e.what());
		return false;
	}
}

/**
 @section Error responses
 */

/**
 * @brief Generates a default HTML error page for the HTTP response.
 *
 * This method constructs a basic HTML page with an error message that includes
 * the HTTP status code and a brief description of the error. The error page is
 * returned as a string and can be used as a fallback response in cases where
 * more specific error content is unavailable.
 *
 * @returns A `std::string` containing the default error page in HTML format.
 */
std::string WsResponseHandler::default_plain_error() {
	std::ostringstream content;
	content << "<!DOCTYPE html>\n"
			<< "<html>\n<head>\n"
			<< "<title>Webserver - Error</title>\n"
			<< "<style>"
			<< BASIC_STYLE << ERROR_STYLE << FOOTER_STYLE
			<< "</style>"
			<< "</head>\n<body>\n"
			<< "<div>"
			<< "<h1>Something went wrong...</h1>\n"
			<< "<h2>" << int_to_string(_request.status) << " - "
			<< http_status_description(_request.status) << "</h2>\n"
			<< "</div>\n"
			<< "</body>\n"
			<< FOOTER_GENERAL << "</html>\n";
	_log->log_debug( RSP_NAME, "Build default error page.");
	return (content.str());
}

/**
 * @brief Sends an HTTP error response to the client, using a custom error page if available.
 *
 * This method attempts to retrieve a custom error page based on the HTTP status code and
 * location configuration. If a custom error page is defined, it loads and sends that page
 * as the error response. If no custom page is available or an error occurs while loading it,
 * a default error page is generated and sent. The response header is constructed according to
 * the status code, content length, and MIME type.
 *
 * If a location was not provided, a plain default text will be sent.
 *
 * @returns `true` if the error response is successfully sent; `false` if an error occurs.
 */
bool WsResponseHandler::send_error_response() {
	std::string error_file;
	std::string file_path = ".html";
	e_http_sts before_file = _request.status;

	if (_location) {
		std::map<int, std::string>::const_iterator it;
		const std::map<int, std::string>* error_pages = &_location->loc_error_pages;
		if (_location->loc_error_mode == TEMPLATE) {
			it = error_pages->find(0);
		} else {
			it = error_pages->find(_request.status);
		}
		if (!error_pages->empty() || it != error_pages->end()) {
			error_file = it->second;
			get_file_content(error_file);
			if (!_response_data.status || _request.status != before_file) {
				_request.status = before_file;
				_log->log_debug( RSP_NAME,
						  "Custom error page cannot be load. http status: " + int_to_string(_request.status));
				_response_data.content = default_plain_error();
			} else {
				_log->log_debug( RSP_NAME, "Custom error page found.");
				file_path = error_file;
				if (_location->loc_error_mode == TEMPLATE)
				{
					_response_data.content = replace_template(_response_data.content, "{error_code}",
															  int_to_string(_request.status));
					_response_data.content = replace_template(_response_data.content, "{error_detail}",
															  http_status_description(_request.status));
					_log->log_debug( RSP_NAME, "Template update to send.");
				}
			}
		} else {
			_log->log_debug( RSP_NAME,
					  "Custom error page was not found. Error: " + int_to_string(_request.status));
			_response_data.content = default_plain_error();
		}
	} else {
		_log->log_debug( RSP_NAME,
		          "Location was not provided. Plain text error will be sent.");
		_response_data.content = default_plain_error();
	}
	_headers = header(_request.status,
					  _response_data.content.length(),
					  get_mime_type(file_path));
	return (sender(_response_data.content));
}

/**
 @section General
 */

/**
 * @brief Handles HTTP redirection for the current request.
 *
 * This method generates an HTTP redirection response based on the redirection information available in the current location configuration (`_location->redirections`).
 * It constructs an HTTP response header indicating the redirection status code and the new location URL, and sends it to the client.
 *
 * @return `true` if the response was successfully sent, otherwise `false`.
 *
 * @details
 * The method follows these steps:
 * 1. **Redirection Configuration**:
 *    - Retrieves the first redirection entry from `_location->redirections` by calling `begin()`.
 *
 * 2. **Generate Redirection Header**:
 *    - Constructs an HTTP response header using a `std::ostringstream`.
 *    - Includes:
 *      - **HTTP Status Line**: Constructs the status line with the status code (`it->first`) and the corresponding reason phrase obtained from `http_status_description()`.
 *      - **Location Header**: Specifies the new location to which the client should be redirected (`it->second`).
 *      - **Content-Length**: Set to `0`, indicating no body content.
 *      - **Content-Type**: Set to `text/html` for compatibility, even though there is no body content.
 *
 * 3. **Send Response**:
 *    - Sets `_headers` to the constructed redirection header.
 *    - Calls `sender()` to send the response to the client, passing an empty string as the body since there is no content.
 *
 * @note
 * - This method assumes that the redirection configuration (`_location->redirections`) is not empty. It uses `begin()` to access the first redirection entry.
 * 	 this is controlled before @see SocketHandler::mapping_redir()
 */
bool WsResponseHandler::redirection() {
	std::ostringstream header;
	const std::map<int, std::string>::const_iterator it = _location->redirections.begin();
	header << "HTTP/1.1 " << it->first << " " << http_status_description((e_http_sts)it->first) << "\r\n"
		   << "Location: " << it->second << "\r\n"
		   << "Content-Length: 0\r\n"
	       << "Content-Type: text/html\r\n"
	       << "\r\n";
	_headers = header.str();

	return (sender(""));
}

/**
 * @brief Disables further request processing by marking the request as invalid.
 *
 * This method is used to set the request's `sanity` flag to `false`, indicating
 * that the request should not continue being processed due to an error. It also
 * sets the HTTP status code for the response, logs the provided error detail,
 * and marks the response data status as invalid.
 *
 * @param status The HTTP status code representing the error condition.
 * @param detail A detailed message describing the reason for marking the request as invalid.
 */
void WsResponseHandler::turn_off_sanity(e_http_sts status, std::string detail) {
	_log->log_error( RSP_NAME, detail);
	_request.sanity = false;
	_request.status = status;
	_response_data.status = false;
}
