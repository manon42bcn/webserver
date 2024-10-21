/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseHandler.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/21 12:53:42 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponseHandler.hpp"

HttpResponseHandler::HttpResponseHandler(int fd, e_http_sts status, e_access access,
										 const LocationConfig *location,
                                         const Logger *log, e_methods method, s_path& path):
		_fd(fd),
		_http_status(status),
		_access(access),
        _location(location),
        _log(log),
        _method(method),
		_resource(path) {
	if (_log == NULL)
		throw Logger::NoLoggerPointer();
	_log->log(LOG_DEBUG, RSP_NAME, "HttpResponseHandler init.");
}

bool HttpResponseHandler::handle_request() {

	switch ((int)_method) {
		case METHOD_GET:
			_log->log(LOG_DEBUG, RSP_NAME, "Handle GET request.");
			return (handle_get());
//		case METHOD_POST:
//			_log->log(LOG_DEBUG, RSP_NAME, "Handle POST request.");
//			handle_post(path);
//			break;
//		case METHOD_DELETE:
//			_log->log(LOG_DEBUG, RSP_NAME, "Handle DELETE request.");
//			handle_delete(path);
//			break;
		default:
			_log->log(LOG_ERROR, RSP_NAME, "Method not allowed.");
			_http_status = HTTP_NOT_IMPLEMENTED;
			send_error_response();
	}
	return (true);
}

/**
 * @brief Handles an HTTP GET request by sending the requested file content.
 *
 * This method processes a GET request by checking the current HTTP status and attempting
 * to load the requested file. If the file is successfully loaded, its content is sent
 * to the client. If there is an error (e.g., the file cannot be found or accessed),
 * an error response is sent instead.
 *
 * @details
 * - The method first checks the `_http_status` to ensure it is `HTTP_OK`. If not, it sends an error response.
 * - It calls `get_file_content()` to retrieve the content of the requested file.
 * - If the file content is successfully loaded, the method sends the content using `sender()`.
 * - If the file cannot be loaded, an error response is sent.
 *
 * @return bool True if the file content was sent successfully, false if an error response was sent.
 */
bool HttpResponseHandler::handle_get() {

	if (_http_status != HTTP_OK || _access < ACCESS_READ) {
		send_error_response();
		return (false);
	}
	s_content content = get_file_content(_resource.path);
	if (content.status) {
		_log->log(LOG_DEBUG, RSP_NAME,
		          "File content will be sent.");
		return (sender(content.content, _resource.path));
	} else {
		_log->log(LOG_ERROR, RSP_NAME,
		          "Get will send a error due to content load fails.");
		return (send_error_response());
	}
}

/**
 * @brief Reads the content of a file from the given path.
 *
 * This method attempts to open and read the specified file in binary mode. If successful,
 * it returns the file's content. If an error occurs (e.g., file cannot be opened, read error),
 * the method logs the error and returns an empty content with a failure status.
 *
 * @details
 * - The method uses `std::ifstream` to open the file in binary mode.
 * - If the file cannot be opened, a log entry is made, and the HTTP status is set to `HTTP_FORBIDDEN`.
 * - The file content is read into a `std::string`, and the method checks whether the file was
 *   completely read. If not, an error is logged, and the HTTP status is set to `HTTP_INTERNAL_SERVER_ERROR`.
 * - Any I/O or general exceptions are caught and handled, with logs being generated for each case.
 *
 * @param path The file system path to the file.
 * @return s_content A structure containing a success flag and the file content.
 */
s_content HttpResponseHandler::get_file_content(const std::string& path) {
	std::string content;
	// Check if path is empty. To avoid further unnecessary errors
	if (path.empty())
		return (s_content(false, ""));

	try {
		std::ifstream file(path.c_str(), std::ios::binary);

		if (!file) {
			_log->log(LOG_ERROR, RSP_NAME,
			          "Failed to open file: " + path);
			_http_status = HTTP_FORBIDDEN;
			return (s_content(false, ""));
		}
		file.seekg(0, std::ios::end);
		std::streampos file_size = file.tellg();
		file.seekg(0, std::ios::beg);
		content.resize(file_size);

		file.read(&content[0], file_size);

		if (!file) {
			_log->log(LOG_ERROR, RSP_NAME,
			          "Error reading file: " + path);
			_http_status = HTTP_INTERNAL_SERVER_ERROR;
			return (s_content(false, ""));
		}
		file.close();
	} catch (const std::ios_base::failure& e) {
		_log->log(LOG_ERROR, RSP_NAME,
		          "I/O failure: " + std::string(e.what()));
		_http_status = HTTP_INTERNAL_SERVER_ERROR;
		return (s_content(false, ""));
	} catch (const std::exception& e) {
		_log->log(LOG_ERROR, RSP_NAME,
		          "Exception: " + std::string(e.what()));
		_http_status = HTTP_INTERNAL_SERVER_ERROR;
		return (s_content(false, ""));
	}
	_log->log(LOG_DEBUG, RSP_NAME,
	          "File content read OK.");
	return (s_content(true, content));
}

/**
 * @brief Generates an HTTP response header.
 *
 * This method constructs a basic HTTP response header based on the provided status code,
 * content size, and MIME type. It follows the HTTP/1.1 specification and includes the
 * status line, `Content-Length`, and `Content-Type` fields.
 *
 * @details
 * - The method returns the header as a string, including the required carriage return
 *   and line feed (`\r\n`) after each line, and an additional `\r\n` at the end to separate
 *   the headers from the content.
 *
 * @param code The HTTP status code (e.g., 200 for OK, 404 for Not Found).
 * @param content_size The size of the content being sent in the response body.
 * @param mime The MIME type of the content (e.g., `text/html`, `application/json`).
 * @return std::string The complete HTTP response header as a string.
 */
std::string HttpResponseHandler::header(int code, size_t content_size, std::string mime) {
	std::ostringstream header;
	header << "HTTP/1.1 " << code << " " << http_status_description((e_http_sts)code) << "\r\n"
	       << "Content-Length: " << content_size << "\r\n"
	       << "Content-Type: " <<  mime << "\r\n"
	       << "Connection: close\r\n"
	       << "\r\n";

	return (header.str());
}

/**
 * @brief Generates a default HTML error page as a string.
 *
 * This method constructs a basic HTML error page incorporating the HTTP status code
 * and its corresponding description. The page includes a simple message indicating
 * that something went wrong, along with the specific error code and description.
 *
 * @details
 * - The HTML content is built by concatenating strings that represent the HTML structure.
 * - The method uses `_http_status`, which should be set prior to calling this method,
 *   to include the relevant status code in the error page.
 * - The `int_to_string()` function converts the status code to a string, and
 *   `http_status_description()` provides a textual description of the status code.
 *
 * @return std::string The complete HTML error page as a string.
 *
 * @note This method is used to provide a fallback error page when a custom error page
 * is not available or cannot be loaded from the configured error pages.
 */
std::string HttpResponseHandler::default_plain_error() {
	std::ostringstream content;
	content << "<!DOCTYPE html>\n"
	        << "<html>\n<head>\n"
	        << "<title>Webserver - Error</title>\n"
	        << "</head>\n<body>\n"
	        << "<h1>Something went wrong...</h1>\n"
	        << "<h2>" << int_to_string(_http_status) << " - "
	        << http_status_description(_http_status) << "</h2>\n"
	        << "</body>\n</html>\n";
	_log->log(LOG_DEBUG, RSP_NAME, "Build default error page.");
	return (content.str());
}

/**
 * @brief Sends an HTTP error response, either using a custom error page or a default message.
 *
 * This method sends an error response to the client, checking for custom error pages defined
 * in the server configuration. If a custom error page is found, it is sent; otherwise, a
 * default plain error message is used. If the error mode is set to `TEMPLATE`, the method
 * replaces template placeholders (e.g., `{error_code}`) with actual values.
 *
 * @details
 * - The method checks the `_location` configuration to see if custom error pages are defined.
 * - It selects the appropriate error page based on the HTTP status code or uses a general template.
 * - If the error mode is `TEMPLATE`, the method replaces placeholders such as `{error_code}`
 *   and `{error_detail}` with the actual HTTP status code and description.
 * - If no custom error page is found, a default plain error message is generated.
 *
 * @return bool True if the response was successfully sent, false otherwise.
 */
bool HttpResponseHandler::send_error_response() {
	std::string content;
	std::string error_file;
	std::string file_path = ".html";

	content = default_plain_error();
	if (_location)
	{
		std::map<int, std::string>::const_iterator it;
		const std::map<int, std::string>* error_pages = &_location->loc_error_pages;
		if (_location->loc_error_mode == TEMPLATE) {
			it = error_pages->find(0);
		} else {
			it = error_pages->find(_http_status);
		}
		if (!error_pages->empty() || it != error_pages->end()) {
			s_content file_content = get_file_content(error_file);
			if (!file_content.status) {
				_log->log(LOG_DEBUG, RSP_NAME,
				          "Custom error page cannot be load. http status: " + int_to_string(_http_status));
				content = default_plain_error();
			} else {
				_log->log(LOG_DEBUG, RSP_NAME, "Custom error page found.");
				content = file_content.content;
				file_path = error_file;
				if (_location->loc_error_mode == TEMPLATE)
				{
					content = replace_template(content, "{error_code}",
					                           int_to_string(_http_status));
					content = replace_template(content, "{error_detail}",
					                           http_status_description(_http_status));
					_log->log(LOG_DEBUG, RSP_NAME, "Template update to send.");
				}
			}
		} else {
			_log->log(LOG_DEBUG, RSP_NAME,
			          "Custom error page was not found. Error: " + int_to_string(_http_status));
		}
	}
	return (sender(content, file_path));
}

/**
 * @brief Sends the HTTP response body in fragments to ensure complete transmission.
 *
 * This method constructs the full HTTP response by generating the headers and appending the body.
 * It then sends the response in fragments, ensuring that all bytes are transmitted to the client.
 *
 * @details
 * - The method handles partial sends by keeping track of how many bytes have been sent.
 * - If `send()` returns `-1` it logs the error and returns `false`.
 * - Any exceptions thrown during the sending process are caught, logged, and result in a `false` return value.
 *
 * @param body The content body to be sent in the response.
 * @param path The file path (or resource) that determines the MIME type of the response.
 * @return bool True if the response was sent successfully, false if an error occurred.
 */
bool HttpResponseHandler::sender(const std::string& body, const std::string& path) {
	try {
		std::string response = header(_http_status, body.length(), get_mime_type(path));
		response += body;
		int total_sent = 0;
		int to_send = (int)response.length();

		while (total_sent < to_send) {
			int sent_bytes = (int)send(_fd, response.c_str() + total_sent, to_send - total_sent, 0);
			if (sent_bytes == -1) {
				_log->log(LOG_ERROR, RSP_NAME, "Error sending the response.");
				return (false);
			}
			total_sent += sent_bytes;
		}
		_log->log(LOG_DEBUG, RSP_NAME, "Response was sent.");
		return (true);
	} catch(const std::exception& e) {
		_log->log(LOG_ERROR, RSP_NAME, e.what());
		return (false);
	}
}
