/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpCGIHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 10:39:21 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/06 20:39:04 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HTTP_CGI_HANDLER_HPP_
#define _HTTP_CGI_HANDLER_HPP_

#include "WebServerResponseHandler.hpp"
#define CGI_NAME "HttpCGIHandler"
#define CGI_TIMEOUT 5000

/**
 * @class HttpCGIHandler
 * @brief Handles HTTP requests that require CGI script execution.
 *
 * The `HttpCGIHandler` class is responsible for executing CGI scripts in response
 * to HTTP requests, capturing their output, and sending the appropriate HTTP responses
 * back to the client. It manages environment setup, process execution, inter-process
 * communication via pipes, and error handling throughout the CGI execution lifecycle.
 *
 * @details
 * - **CGI Execution**: Executes external scripts or programs as per the CGI specification.
 * - **Environment Management**: Sets up and cleans up environment variables required by the CGI script.
 * - **Process Handling**: Manages child processes, including fork and exec operations.
 * - **Response Parsing**: Parses the output from the CGI script to construct valid HTTP responses.
 * - **Error Handling**: Provides robust error detection and handling mechanisms to ensure server stability.
 *
 * ### Public Methods
 * - `HttpCGIHandler(...)`: Constructor that initializes the handler with necessary configurations.
 * - `~HttpCGIHandler()`: Destructor that cleans up resources.
 * - `bool handle_request()`: Main method to process the CGI request.
 *
 * ### Private Methods
 * - `bool cgi_execute()`: Executes the CGI script and manages pipes and processes.
 * - `void get_file_content(int pid, int (&fd)[2])`: Reads the CGI output from the pipe.
 * - `std::vector<char*> cgi_environment()`: Sets up the CGI environment variables.
 * - `void free_cgi_env()`: Frees the allocated environment variables.
 * - `bool send_response(const std::string &body, const std::string &path)`: Sends the response to the client.
 *
 * @note This class is implemented as part of WebServer Project. Some errors controls are
 * 		 part of previous process.
 */
class HttpCGIHandler : public WsResponseHandler {
	private:
		std::vector<char*>          _cgi_env;

		bool cgi_execute();
		void get_file_content(int pid, int (&fd)[2]);
		std::vector<char*> cgi_environment();
		void free_cgi_env();
		bool send_response(const std::string &body, const std::string &path);
	public:
		HttpCGIHandler(const LocationConfig *location,
							const Logger *log,
							ClientData* client_data,
							s_request& request,
							int fd);
		~HttpCGIHandler();
		bool handle_request();
};

#endif
