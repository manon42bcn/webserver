/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRangeHandler.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/07 09:37:41 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/07 09:37:41 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HTTP_RANGE_HANDLER_HPP_
#define _HTTP_RANGE_HANDLER_HPP_

#include "WebServerResponseHandler.hpp"
#define RRH_NAME "HttpRangeHandler"

/**
 * @class HttpRangeHandler
 * @brief Manages HTTP requests that include range-specific content, such as partial file downloads.
 *
 * This class extends `WsResponseHandler` to handle HTTP requests with a range header, enabling
 * partial content retrieval for efficient file handling and data streaming.
 *
 * @details
 * - `HttpRangeHandler` processes requests with range headers, validates content range,
 *   and reads file content based on the specified range.
 * - Supports GET requests with range validation for files and manages content responses.
 * - Relies on configuration from `LocationConfig` and logs actions via `Logger`.
 *
 * ### Public Methods
 * - `HttpRangeHandler(const LocationConfig *location, const Logger *log, ClientData* client_data, s_request& request, int fd)`: Constructor initializing the range handler with configuration, logger, and client data.
 * - `~HttpRangeHandler()`: Destructor to clean up resources, particularly file descriptors.
 * - `bool handle_request()`: Processes the HTTP request, validating the range and retrieving file content.
 *
 * ### Private Methods
 * - `bool handle_get()`: Handles GET requests, validating and serving the requested content range.
 * - `void get_file_content(std::string& path)`: Reads the specified file content for the range.
 * - `bool validate_content_range(size_t file_size)`: Ensures the specified content range is valid within the file's total size.
 * - `void parse_content_range()`: Parses the range header to determine the requested content range.
 * - `void get_file_content(int pid, int (&fd)[2])`: Reads file content within a child process for specific cases, managing pipes.
 *
 */
class HttpRangeHandler : public WsResponseHandler {
	public:
		HttpRangeHandler(const LocationConfig *location,
	                     const Logger *log,
	                     ClientData* client_data,
	                     s_request& request,
	                     int fd);
		bool handle_request();
	private:
		bool handle_get();
		void get_file_content(int pid, int (&fd)[2]);
		void get_file_content(std::string& path);
		void parse_content_range();
		bool validate_content_range(size_t file_size);
};

#endif
