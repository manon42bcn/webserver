/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpCGIHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 10:39:37 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/06 14:34:37 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpCGIHandler.hpp"

/**
 * @brief Constructs the `HttpCGIHandler` object, initializing it with request
 *        details and logging the initialization.
 *
 * This constructor initializes an `HttpCGIHandler` instance to handle CGI
 * requests within the server. It inherits initialization parameters and behavior
 * from the `WsResponseHandler` class, which provides essential elements for
 * response handling, including the configuration, logging, client data, and
 * request details.
 *
 * @param location Pointer to a `LocationConfig` object that provides the
 *        configuration for the CGI execution environment.
 * @param log Pointer to the `Logger` object for logging server activities
 *        and CGI-specific events.
 * @param client_data Pointer to the `ClientData` structure containing
 *        information about the connected client.
 * @param request Reference to the `s_request` structure that encapsulates
 *        request data, including headers, body, and method details.
 * @param fd The file descriptor associated with the client, used for
 *        sending and receiving data.
 *
 * @note The constructor logs the initialization of the CGI handler at the
 *       DEBUG level to indicate successful setup.
 */
HttpCGIHandler::HttpCGIHandler(const LocationConfig *location,
							   const Logger *log,
							   ClientData* client_data,
							   s_request& request,
							   int fd) :
							   WsResponseHandler(location, log, client_data,
												 request, fd)
{
	_log->log(LOG_DEBUG, CGI_NAME,
			  "Cgi Handler init.");
}

/**
 * @brief Handles the CGI request and processes the response headers and body.
 *
 * This method is responsible for handling and processing the response
 * from a CGI (Common Gateway Interface) script. The CGI is executed, and its
 * output, which includes HTTP headers and the body content, is read and parsed.
 * This method ensures the response includes essential headers and verifies their
 * validity before constructing the HTTP response for the client.
 *
 * @details
 * Steps of operation:
 * - Executes the CGI script and stores the output in `_response_data.content`.
 * - Verifies that the CGI response is not empty. If it is, an HTTP 502 (Bad Gateway)
 *   error is sent to the client.
 * - Finds the end of the headers in the CGI response content using the
 *   `end_of_header_system()` helper function. If a valid header boundary is
 *   not found, the function considers it an invalid CGI response.
 * - Parses and verifies the presence of the "Content-Type" header:
 *   - The "Content-Type" header is required for CGI responses to indicate
 *     the type of data being sent to the client.
 *   - If this header is missing, an HTTP 500 (Internal Server Error) is sent.
 * - Parses and verifies the presence of an HTTP status header:
 *   - The CGI can optionally specify an HTTP status using the "Status" header.
 *   - If a valid "Status" header is not found, the status defaults to 200 OK.
 *   - If an invalid status code is provided, an HTTP 502 (Bad Gateway) is sent.
 * - Extracts and removes the CGI headers from `_response_data.content`, leaving only
 *   the body, which will be sent to the client.
 * - Constructs the final HTTP response headers, including the appropriate
 *   status and content type, and sends the complete response to the client.
 *
 * @return true if the CGI request was successfully handled and the response was
 *         prepared and sent to the client.
 * @return false if there was an error in handling the CGI request, in which
 *         case an error response is sent to the client.
 */
bool HttpCGIHandler::handle_request() {
	cgi_execute();
	if (!_response_data.content.empty()) {
		size_t header_pos = end_of_header_system(_response_data.content);
		if (header_pos == std::string::npos) {
			turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
			                "CGI Response does not include a valid header.");
			send_error_response();
			return (false);
		}
		_response_data.mime = get_header_value(_response_data.content, "content-type:", "\n");
		if (_response_data.mime.empty()){
			turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
			                "Content-Type not present at CGI response.");
			send_error_response();
			return (false);
		}
		std::string status = get_header_value(_response_data.content, "status:", " ");
		if (status.empty()) {
			_response_data.http_status = HTTP_OK;
			_request.status = HTTP_OK;
			std::ostringstream header;
			header << "HTTP/1.1 " << HTTP_OK << " " << http_status_description(HTTP_OK) << "\r\n"
			       << _response_data.content.substr(0,header_pos) << "\r\n";
			_headers = header.str();
		} else {
			int http_status = atoi(status.c_str());
			if (http_status_description((e_http_sts)http_status) != "No Info Associated") {
				_headers = _response_data.content.substr(0,header_pos) + "\r\n";
				_response_data.http_status = (e_http_sts)http_status;
			} else {
				turn_off_sanity(HTTP_BAD_GATEWAY,
				                "Non valid HTTP status provided by CGI.");
				send_error_response();
				return (false);
			}
		}
		_response_data.content = _response_data.content.substr(header_pos + 1);
		send_response(_response_data.content, _request.normalized_path);
		return (true);
	} else {
		turn_off_sanity(HTTP_BAD_GATEWAY,
		                "CGI does not include a response.");
		send_error_response();
		return (false);
	}
}

/**
 * @brief Destructor to accomplish Abstract class.
 */
HttpCGIHandler::~HttpCGIHandler () {}

/**
 * @brief Executes the CGI script, handling input and output through pipes.
 *
 * This method sets up the environment and pipes needed to execute a CGI
 * script and manages the data flow between the server and the CGI process.
 * It ensures proper redirection of input and output streams for the CGI and
 * processes the result accordingly.
 *
 * @details
 * Steps of operation:
 * - Creates two pipes (`cgi_in` and `cgi_out`) to handle the input and output
 *   for the CGI process.
 *   - `cgi_in` will be used to send the request body to the CGI's standard input.
 *   - `cgi_out` will be used to capture the CGI’s output to standard output.
 * - Calls `fork()` to create a new process:
 *   - If `fork()` fails, it logs the error, closes all pipes, frees environment
 *     variables, and sets an HTTP 500 (Internal Server Error).
 *   - In the child process:
 *     - Redirects `STDIN_FILENO` to `cgi_in[0]` to allow reading from the pipe.
 *     - Redirects `STDOUT_FILENO` to `cgi_out[1]` to capture the output from CGI.
 *     - Closes unused ends of the pipes to prevent unwanted I/O operations.
 *     - Executes the CGI script using `execve`, passing the path, arguments,
 *       and environment variables.
 *     - If `execve` fails, logs the error and exits the child process.
 *   - In the parent process:
 *     - Closes the unused pipe ends and writes the request body to `cgi_in[1]`
 *       (CGI’s standard input), if present.
 *     - Closes `cgi_in[1]` after writing to signal end of input.
 *     - Calls `get_file_content()` to read the CGI output from `cgi_out`.
 *     - Closes pipes and frees environment resources.
 * - Error handling includes cleanup of pipes and environment variables in case
 *   of unexpected issues, and logging error details.
 *
 * @return true if the CGI request was successfully executed and processed.
 * @return false if there was an error in handling the CGI request, in which case
 *         an error response is prepared for the client.
 */
bool HttpCGIHandler::cgi_execute() {
	int cgi_in[2];
	int cgi_out[2];

	if (pipe(cgi_in) == -1 || pipe(cgi_out) == -1) {
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
		                "Error building pipes to CGI handle.");
		return (false);
	}
	_cgi_env = cgi_environment();
	if (!_request.sanity) {
		return (false);
	}
	try {
		pid_t pid = fork();
		if (pid == -1) {
			turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
			                "Fork process error.");
			close(cgi_in[0]);
			close(cgi_in[1]);
			close(cgi_out[0]);
			close(cgi_out[1]);
			free_cgi_env();
			return (false);
		} else if (pid == 0) {
			if (dup2(cgi_in[0], STDIN_FILENO) == -1 || dup2(cgi_out[1], STDOUT_FILENO) == -1) {
				_log->log(LOG_ERROR, RSP_NAME,
						  "Error duplicating file descriptor.");
				exit(1);
			}
			close(cgi_in[1]);
			close(cgi_out[0]);
			close(cgi_in[0]);
			close(cgi_out[1]);

			std::string cgi_path = _request.normalized_path + _request.script;
			char* const argv[] = { const_cast<char*>(cgi_path.c_str()), NULL };
			execve(cgi_path.c_str(), argv, _cgi_env.data());
			_log->log(LOG_ERROR, RSP_NAME,
			          "execve function error.");
			exit(1);
		} else {
			close(cgi_in[0]);
			close(cgi_out[1]);
			if (!_request.body.empty()) {
				ssize_t written = write(cgi_in[1], _request.body.c_str(), _request.body.size());
				if (written == -1) {
					_log->log(LOG_ERROR, RSP_NAME,
					          "Error writing to CGI input.");
				}
			}
			close(cgi_in[1]);
			get_file_content(pid, cgi_out);
			close(cgi_in[0]);
			free_cgi_env();
			if (!_response_data.status) {
				send_error_response();
			}
			return (true);
		}
	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Unexpected Error at pipe execution.: " << e.what();
		free_cgi_env();
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
						detail.str());
		return (false);
	}
}

/**
 * @brief Reads the output from a CGI process through a non-blocking pipe.
 *
 * This method reads the response from a CGI script using a non-blocking pipe
 * and waits for data to become available with a timeout mechanism.
 * The method ensures that the CGI script does not block the server indefinitely
 * by using `poll()` to monitor the pipe for input and to implement a timeout.
 *
 * @details
 * Steps of operation:
 * - Sets the CGI output pipe (fd[0]) to non-blocking mode using `fcntl`.
 * - Initializes a `pollfd` structure and monitors the pipe for readable events (`POLLIN`).
 * - Enters a loop where it waits for input on the pipe with a specified timeout (`CGI_TIMEOUT`):
 *   - If the pipe closes from the CGI side (`POLLHUP`), it logs this and exits the loop.
 *   - If the timeout expires, it logs an HTTP 504 (Gateway Timeout) error, sets `_response_data.status`
 *     to indicate an error, and exits.
 *   - If an error occurs in `poll`, it logs an HTTP 500 (Internal Server Error) and exits.
 *   - If input is available (`POLLIN`), it reads the available data into a buffer:
 *     - If `read` returns data, it appends this to `response`.
 *     - If `read` returns `0`, indicating EOF, it breaks out of the loop.
 *     - If `read` returns an error, it retries for `EINTR` or `EAGAIN` errors. For other errors,
 *       it logs an HTTP 500 (Internal Server Error) and exits the loop.
 * - If `_response_data.status` is false after the loop, it kills the CGI process (`SIGKILL`) and
 *   calls `waitpid` to clean up the process resources.
 * - Once the data is fully read or an error occurs, the output is stored in `_response_data.content`.
 *
 * @param pid Process ID of the CGI script, which is killed if a timeout or critical error occurs.
 * @param fd An array representing the CGI pipe; `fd[0]` is used for reading the CGI's output.
 */
void HttpCGIHandler::get_file_content(int pid, int (&fd)[2]) {
	char buffer[2048];
	std::string response;
	ssize_t bytes_read;
	_response_data.status = true;

	fcntl(fd[0], F_SETFL, O_NONBLOCK);
	struct pollfd pfd;
	pfd.fd = fd[0];
	pfd.events = POLLIN;
	_log->log(LOG_DEBUG, RSP_NAME,
			  "Reading CGI response.");
	while (true) {
		int poll_result = poll(&pfd, 1, CGI_TIMEOUT);
		if (pfd.revents & POLLHUP) {
			_log->log(LOG_DEBUG, RSP_NAME,
					  "CGI pipe closed by writer.");
			break;
		}
		if (poll_result == 0) {
			turn_off_sanity(HTTP_GATEWAY_TIMEOUT,
							"CGI Timeout.");
			break;
		} else if (poll_result == -1) {
			turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
							"Poll error on CGI pipe.");
			break;
		}
		if (pfd.revents & POLLIN) {
			bytes_read = read(fd[0], buffer, sizeof(buffer));
			if (bytes_read > 0) {
				response.append(buffer, bytes_read);
			} else if (bytes_read == 0) {
				break;
			} else {
				if (errno == EINTR || errno == EAGAIN) {
					continue ;
				}
				turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
								"Error reading CGI response.");
				break;
			}
		}
	}
	if (!_response_data.status) {
		kill(pid, SIGKILL);
		waitpid(pid, NULL, WNOHANG);
		_log->log(LOG_WARNING, CGI_NAME,
				  "CGI process was kill due to a timeout error.");
	}
	waitpid(pid, NULL, 0);
	_response_data.content = response;
}

/**
 * @brief Prepares the CGI environment variables for the CGI process.
 *
 * This method builds a set of environment variables as required by the CGI
 * specification. It assembles these environment variables into a `std::vector`
 * of `char*`, formatted as expected by the `execve` system call. Each variable
 * is allocated dynamically to ensure compatibility with the `execve` requirements.
 *
 * @details
 * Steps of operation:
 * - Fills a `std::vector<std::string>` with CGI-required environment variables,
 *   setting values based on the current request properties:
 *   - `GATEWAY_INTERFACE`: CGI version (hardcoded as "CGI/1.1").
 *   - `SERVER_PROTOCOL`: HTTP version (hardcoded as "HTTP/1.1").
 *   - `REQUEST_METHOD`: HTTP method of the request (e.g., GET, POST).
 *   - `QUERY_STRING`: Any query string from the URL.
 *   - `CONTENT_TYPE`: MIME type of the request content, if available.
 *   - `CONTENT_LENGTH`: Length of the request body, if provided.
 *   - `PATH_INFO`: Path info derived from the request URL.
 *   - `SCRIPT_NAME`: Name of the CGI script.
 *   - `SERVER_NAME`: Server name as specified in the configuration.
 *   - `SERVER_PORT`: Server port handling the request.
 * - Converts each environment variable to `char*` using `strdup` and stores
 *   pointers in a `std::vector<char*>` for compatibility with `execve`.
 * - Appends a final `NULL` pointer to signal the end of the environment list.
 * - Returns the vector of environment variables, leaving responsibility for
 *   freeing memory to the caller.
 *
 * @return A `std::vector<char*>` containing environment variables as `char*`
 *         strings. The caller is responsible for freeing each element.
 */
std::vector<char*> HttpCGIHandler::cgi_environment() {
	std::vector<std::string> env_vars;

	env_vars.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env_vars.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env_vars.push_back("REQUEST_METHOD=" + method_enum_to_string(_request.method));
	env_vars.push_back("QUERY_STRING=" + _request.query);
	env_vars.push_back("CONTENT_TYPE=" + _request.content_type);
	env_vars.push_back("CONTENT_LENGTH=" + int_to_string((int)_request.content_length));
	env_vars.push_back("PATH_INFO=" + _request.path_info);
	env_vars.push_back("SCRIPT_NAME=" + _request.script);
	env_vars.push_back("SERVER_NAME=" + _client_data->get_server()->get_config().server_name);
	env_vars.push_back("SERVER_PORT=" + _client_data->get_server()->get_port());
	std::vector<char*> env_ptrs;
	try {
		for (size_t i = 0; i < env_vars.size(); ++i) {
			char* env_entry = strdup(env_vars[i].c_str());
			if (!env_entry) {
				for (size_t j = 0; j < env_ptrs.size(); ++j) {
					free(env_ptrs[j]);
				}
				turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
								"Error Allocating env vars.");
			}
			env_ptrs.push_back(env_entry);
		}
		env_ptrs.push_back(NULL);
	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Failed to set up CGI environment variables.: " << e.what();
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
		                detail.str());
	}
	return (env_ptrs);
}

/**
 * @brief Frees the dynamically allocated memory in the CGI environment vector.
 *
 * This method iterates through `_cgi_env`, freeing each dynamically allocated
 * `char*` entry created by `strdup` in `cgi_environment`. It skips the last
 * element (NULL pointer) to avoid undefined behavior.
 *
 * @details
 * - Assumes that `_cgi_env` was populated by `cgi_environment`, which uses
 *   `strdup` to create each `char*` entry.
 * - Frees each `char*` in `_cgi_env` except for the last NULL pointer.
 */
void HttpCGIHandler::free_cgi_env() {
	for (size_t i = 0; i < _cgi_env.size() - 1; ++i) {
		free(_cgi_env[i]);
	}
}

/**
 * @brief Sends the HTTP response to the client, using the specified body.
 *
 * This method prepares the response to be sent to the client by calling
 * `sender` with the body content. It is designed to handle the final response
 * preparation and delivery, making it easily callable after processing the
 * CGI response. Due to CGI will return its own headers, this method is
 * overload to avoid extra checks here, and base one.
 *
 * @param body The content to be sent as the HTTP response body.
 * @param path The path associated with the response (unused here).
 * @return true if the response was successfully sent.
 * @return false if an error occurred in sending the response.
 */
bool HttpCGIHandler::send_response(const std::string &body, const std::string &path) {
	UNUSED(path);
	return(sender(body));
}
