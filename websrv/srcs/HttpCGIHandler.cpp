/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpCGIHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 10:39:37 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/23 22:06:22 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpCGIHandler.hpp"

/**
 * @brief Constructs an `HttpCGIHandler` for processing CGI requests.
 *
 * Initializes the CGI handler with the given location configuration, logger,
 * client data, request details, and file descriptor. It logs the initialization
 * process and ensures that critical pointers are valid.
 *
 * @param location Pointer to the `LocationConfig` containing server configuration.
 * @param log Pointer to the `Logger` instance for logging activities.
 * @param client_data Pointer to `ClientData` containing client connection details.
 * @param request Reference to the `s_request` structure with the request information.
 * @param fd File descriptor associated with the client connection.
 *
 */
HttpCGIHandler::HttpCGIHandler(const LocationConfig *location,
							   const Logger *log,
							   ClientData* client_data,
							   s_request& request,
							   int fd) :
							   WsResponseHandler(location, log, client_data,
												 request, fd)
{
	_log->log_debug( CGI_NAME,
			  "Cgi Handler init.");
}

/**
 * @brief Destructor for `HttpCGIHandler`, responsible for cleaning up resources.
 *
 * This destructor frees the allocated CGI environment variables and logs the cleanup process.
 * It ensures that no exceptions escape the destructor to maintain exception safety.
 */
HttpCGIHandler::~HttpCGIHandler () {
	try {
		free_cgi_env();
		_log->log_debug( CGI_NAME,
		          "CGI envs freed.");
	} catch (std::exception& e) {
		std::ostringstream details;
		details << "Error during CGI Destructor: " << e.what();
		_log->log_warning( CGI_NAME,
		          details.str());
	}
}


/**
 * @brief Handles the CGI request by executing the CGI script and sending the response.
 *
 * This method executes the CGI script, processes its output, and constructs an HTTP response
 * to be sent back to the client. It performs validation on the CGI output, ensuring that
 * necessary headers like `Content-Type` are present.
 *
 * @return `true` if the response is successfully sent; `false` if an error occurs.
 *
 * @details
 * - **CGI Execution**: Calls `cgi_execute()` to run the CGI script.
 * - **Response Validation**: Checks if the CGI script produced any output.
 * - **Header Parsing**: Extracts headers and status code from the CGI response.
 * - **Error Handling**: Sends appropriate error responses if validation fails.
 */
bool HttpCGIHandler::handle_request() {
//	TODO: This is a workaround to fix some path issues
	if (_request.normalized_path[_request.normalized_path.size() - 1] != '/') {
		_request.normalized_path += "/";
	}
	if (!cgi_execute()) {
		send_error_response();
		return (false);
	}
	if (!_response_data.content.empty()) {
		size_t header_pos = end_of_header_system(_response_data.content);
		if (header_pos == std::string::npos) {
			turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
			                "CGI Response does not include a valid header.");
			send_error_response();
			return (false);
		}
		_response_data.mime = get_header_value(_response_data.content,
											   "content-type:");
		if (_response_data.mime.empty()){
			turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
			                "Content-Type not present at CGI response.");
			send_error_response();
			return (false);
		}
		std::string status = get_header_value(_response_data.content,
											  "status:", " ");

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
		std::string keep = get_header_value(_headers, "connection:");
		if (keep.empty()) {
			_headers += "Connection: close\r\n";
		}
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
 * @brief Executes the CGI script by setting up pipes and handling the process execution.
 *
 * This method creates input and output pipes for communication with the CGI script,
 * forks a child process to execute the script, and manages data exchange between
 * the server and the CGI process. It handles errors related to pipe creation,
 * forking, and execution, ensuring resources are properly cleaned up.
 *
 * @return `true` if the CGI execution is successful; `false` if an error occurs.
 *
 * @details
 * - **Pipe Setup**: Creates pipes `cgi_in` and `cgi_out` for CGI communication.
 * - **Environment Setup**: Prepares the CGI environment variables.
 * - **Forking Process**:
 *   - **Child Process**: Duplicates file descriptors, executes the CGI script using `execve`.
 *   - **Parent Process**: Writes request body to the CGI input pipe, reads the output.
 * - **Error Handling**: Cleans up file descriptors and environment variables on errors.
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
				_log->log_error( CGI_NAME,
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
			_log->log_error( CGI_NAME,
			          "execve function error.");
			exit(1);
		} else {
			close(cgi_in[0]);
			close(cgi_out[1]);
			if (!_request.body.empty()) {
				ssize_t written = write(cgi_in[1], _request.body.c_str(), _request.body.size());
				if (written == -1) {
					_log->log_error( CGI_NAME,
					          "Error writing to CGI input.");
				}
			}
			close(cgi_in[1]);
			get_file_content(pid, cgi_out);
			close(cgi_in[0]);
			if (!_request.sanity) {
				return (false);
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
 * @brief Reads the CGI script output from the pipe and handles timeouts.
 *
 * This method reads the CGI script's output using non-blocking I/O and `poll` to
 * handle potential timeouts. It assembles the output into a response string and
 * ensures that the CGI process is properly terminated in case of errors or timeouts.
 *
 * @param pid The process ID of the CGI child process.
 * @param fd Array of file descriptors for the output pipe.
 *
 * @details
 * - **Non-Blocking Read**: Sets the file descriptor to non-blocking mode.
 * - **Polling**: Uses `poll()` to wait for data with a timeout defined by `CGI_TIMEOUT`.
 * - **Data Reading**: Reads data from the pipe when available and appends to the response.
 * - **Process Termination**: Kills the CGI process if a timeout or error occurs.
 * - **Resource Cleanup**: Closes file descriptors and waits for the child process to prevent zombies.
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
	_log->log_debug( CGI_NAME,
			  "Reading CGI response.");
	while (true) {
		int poll_result = poll(&pfd, 1, CGI_TIMEOUT);
		if (pfd.revents & POLLHUP) {
			_log->log_debug( CGI_NAME,
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
		_log->log_warning( CGI_NAME,
				  "CGI process was kill due to a timeout error.");
	}
	waitpid(pid, NULL, 0);
	_response_data.content = response;
}

/**
 * @brief Sets up the environment variables required for CGI execution.
 *
 * This method prepares the necessary environment variables according to the CGI specification,
 * converting them into a format suitable for `execve`. It handles memory allocation and ensures
 * proper cleanup in case of errors.
 *
 * @return A vector of C-style strings (`char*`) representing the environment variables.
 *
 * @details
 * - **Environment Variables**: Populates variables such as `REQUEST_METHOD`, `CONTENT_LENGTH`, `SCRIPT_NAME`, etc.
 * - **Memory Allocation**: Uses `strdup` to allocate memory for each environment string.
 * - **Error Handling**: If allocation fails, it disables sanity and frees any allocated memory.
 * - **Null-Termination**: Ensures the environment array is null-terminated as required by `execve`.
 */
std::vector<char*> HttpCGIHandler::cgi_environment() {
	const ServerConfig* host = _request.host_config;
	std::vector<std::string> env_vars;

	env_vars.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env_vars.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env_vars.push_back("HTTP_COOKIE=" + _request.cookie);
	env_vars.push_back("REQUEST_METHOD=" + _request.method_str);
	env_vars.push_back("QUERY_STRING=" + _request.query);
	env_vars.push_back("CONTENT_TYPE=" + _request.content_type);
	env_vars.push_back("CONTENT_LENGTH=" + int_to_string((int)_request.content_length));
	env_vars.push_back("PATH_INFO=" + _request.path_info);
	env_vars.push_back("SCRIPT_NAME=" + _request.script);
	env_vars.push_back("SERVER_NAME=" + host->server_name);
	env_vars.push_back("SERVER_NAME=" + host->server_name);
	env_vars.push_back("SERVER_PORT=" + host->server_name);
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
 * @brief Frees the allocated CGI environment variables.
 *
 * This method iterates over the `_cgi_env` vector and frees each allocated string.
 * It then clears the vector to remove any dangling pointers.
 */
void HttpCGIHandler::free_cgi_env() {
	for (size_t i = 0; i < _cgi_env.size() - 1; ++i) {
		free(_cgi_env[i]);
	}
}

/**
 * @brief Sends the CGI response body to the client.
 *
 * This method invokes the `sender` function to transmit the response body to the client.
 * The `path` parameter is unused in this context.
 *
 * @param body The response body content to be sent.
 * @param path The file path (unused in this method).
 * @return `true` if the response is successfully sent; `false` otherwise.
 */
bool HttpCGIHandler::send_response(const std::string &body, const std::string &path) {
	UNUSED(path);
	return(sender(body));
}
