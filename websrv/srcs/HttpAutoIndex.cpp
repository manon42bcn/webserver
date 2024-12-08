/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpAutoIndex.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/17 22:09:10 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/20 22:59:02 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpAutoIndex.hpp"

/**
 * @brief Constructs a new `HttpAutoIndex` object.
 *
 * This constructor initializes an `HttpAutoIndex` instance to handle auto-indexing of directory requests. It calls the base class `WsResponseHandler` constructor to initialize common attributes, and then logs the initialization of the `HttpAutoIndex` handler.
 *
 * @param location A pointer to the `LocationConfig` object containing location-specific settings.
 * @param log A pointer to the `Logger` object for logging purposes.
 * @param client_data A pointer to the `ClientData` object containing data for the current client.
 * @param request A reference to the `s_request` structure containing the current request data.
 * @param fd The file descriptor used for managing the connection.
 *
 * @details
 * The constructor performs the following actions:
 * 1. Calls the constructor of the base class `WsResponseHandler` to initialize the necessary attributes (`location`, `log`, `client_data`, `request`, and `fd`).
 * 2. Logs a debug message indicating the successful initialization of the `HttpAutoIndex` handler.
 */
HttpAutoIndex::HttpAutoIndex(const LocationConfig *location,
							 const Logger *log,
							 ClientData *client_data,
							 s_request &request,
							 int fd):
							 WsResponseHandler(location, log, client_data, request, fd) {
	_log->log_debug( AI_NAME,
			  "Auto Index Handler init.");
}

/**
 * @brief Handles an HTTP GET request for auto-generating a directory index.
 *
 * This method processes an HTTP GET request to generate an automatic index page for a directory.
 * It first checks if the GET method is allowed on the requested resource. If not allowed, it sends an HTTP 403 Forbidden response.
 * If GET is allowed, it attempts to load the content of the directory by calling `get_file_content()`.
 * Depending on the success of the file content loading, it either sends the generated HTML response or returns an error response.
 *
 * @return `true` if the request is handled successfully and the response is sent, otherwise `false`.
 *
 * @details
 * The method follows these main steps:
 * 1. **Allowed Method Check**:
 *    - Checks if the HTTP GET method is allowed for the current resource by evaluating `_location->loc_allowed_methods`.
 *    - If GET is not allowed, the method sets the response status to HTTP 403 (Forbidden) and sends an error response.
 *    - It then returns `false` indicating that the request was not handled successfully.
 *
 * 2. **Loading File Content**:
 *    - If GET is allowed, it calls `get_file_content()` to load the content of the directory indicated by `_request.normalized_path`.
 *    - If the directory content is successfully loaded (`_response_data.status` is `true`), it sets the MIME type to `text/html`.
 *    - Logs a debug message indicating that the file content will be sent.
 *    - Sends the response containing the generated directory index using `send_response()`.
 *    - Returns `true` if the response was successfully sent.
 *
 * 3. **Error Handling**:
 *    - If `get_file_content()` fails to load the directory content, it logs a debug message.
 *    - Sends an error response using `send_error_response()`.
 *    - Returns `false` indicating that an error occurred while handling the request.
 */
bool HttpAutoIndex::handle_request() {
	if (!HAS_GET(_location->loc_allowed_methods)) {
		turn_off_sanity(HTTP_FORBIDDEN,
		                "Get not allowed over resource.");
		send_error_response();
		return (false);
	}
	get_file_content(_request.normalized_path);
	if (_response_data.status) {
		_response_data.mime = "text/html";
		_log->log_debug( RSP_NAME,
		                 "File content will be sent.");
		return (send_response(_response_data.content, _request.normalized_path));
	}
	_log->log_debug( RSP_NAME,
	                 "Get will send a error due to content load fails.");
	return (send_error_response());
}

/**
 * @brief Generates the content of an auto index for a given directory.
 *
 * This method reads the contents of a specified directory and generates an HTML page that serves as an automatic index.
 * It lists all the files and directories in the provided directory path, displaying details such as the file name, size, and last modified date.
 *
 * @param path The path to the directory for which the auto index will be generated.
 *
 * @details
 * The method follows these steps:
 * 1. **Open Directory**:
 *    - Attempts to open the directory specified by the `path`.
 *    - If the directory cannot be opened, it logs an error, sets the HTTP status to `HTTP_NOT_FOUND`, and exits the function.
 *
 * 2. **Generate HTML Content**:
 *    - Constructs the initial HTML structure, including the title, styles, and a heading indicating the directory being indexed.
 *    - Iterates over each entry in the directory, excluding `.` and `..`.
 *    - For each entry, constructs a table row containing:
 *      - A link to the file or directory.
 *      - The size (in bytes) if it is a file or the label `Directory` if it is a directory.
 *      - The last modified date in the format `YYYY-MM-DD HH:MM:SS`.
 *
 * 3. **Error Handling**:
 *    - If an error occurs while reading a file's information, logs an error and continues with the next entry.
 *
 * 4. **Close Directory and Set Response**:
 *    - After iterating through all entries, the directory is closed, and the generated HTML content is stored in `_response_data.content`.
 *    - Sets `_response_data.status` to `true` to indicate that the content generation was successful.
 *
 * 5. **Exception Handling**:
 *    - If an exception is thrown during the process, logs a warning message, sets the HTTP status to `HTTP_INTERNAL_SERVER_ERROR`, and sets `_response_data.status` to `false`.
 *
 * @note This method generates a complete HTML page with a table format to present the directory content in a user-friendly way.
 */
void HttpAutoIndex::get_file_content(std::string& path) {
	try {
		DIR* dir = opendir(path.c_str());
		if (dir == NULL) {
			turn_off_sanity(HTTP_NOT_FOUND,
			                "Dir cannot be open.");
			return ;
		}
		std::string path_to_file = _request.path;
		if (!_location->is_root && !_request.path_request.empty()) {
			path_to_file = _request.path_request;
		}
		if (path_to_file[path_to_file.size() - 1] != '/') {
			path_to_file += "/";
		}
		std::ostringstream out;
		out << "<!DOCTYPE html>\n<html>\n<head>\n<title>Index of " << _request.path << "</title>\n";
		out << "<style>" << BASIC_STYLE << AUTOINDEX_STYLE << FOOTER_STYLE << "</style>\n";
		out << "</head>\n<body>\n";
		out << "<div>";
		out << "<h1 class=\"autoindex\">Index of : " << _request.path << "</h1>\n";
		out << "<table>\n<tr><th>Name</th><th>Size</th><th>Last Modified</th></tr>\n";

		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL) {
			std::string name = entry->d_name;
			if (name == "." || name == "..") {
				continue;
			}
			std::string base_path = _request.normalized_path + name;
			struct stat file_stat;
			if (stat(base_path.c_str(), &file_stat) == -1) {
				_log->log_warning(AI_NAME,
				                "Error reading file.");
				continue;
			}
			out << "<tr>";
			out << "<td><a href=\"" << path_to_file << name << "\">" << name << "</a></td>";
			if (S_ISDIR(file_stat.st_mode)) {
				out << "<td>Directory</td>";
			} else {
				std::ostringstream size_stream;
				size_stream << file_stat.st_size;
				out << "<td>" << size_stream.str() << " bytes</td>";
			}
			char time_buf[80];
			std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", std::localtime(&file_stat.st_mtime));
			out << "<td>" << time_buf << "</td>";

			out << "</tr>\n";
		}

		out << "</table>\n</div>\n" << FOOTER_GENERAL << "</body>\n</html>";
		closedir(dir);
		_response_data.content = out.str();
		_response_data.status = true;
	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Error Creating Autoindex file for " << path << ". Error: " << e.what();
		_log->log_warning(AI_NAME,detail.str());
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
						"Error Creating Autoindex data.");
		_response_data.status = false;
	}
}

void HttpAutoIndex::get_file_content(int pid, int (&fd)[2]) {
	UNUSED(pid);
	UNUSED(fd);
}
