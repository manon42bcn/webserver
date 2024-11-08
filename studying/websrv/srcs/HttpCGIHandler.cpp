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

bool HttpCGIHandler::handle_request() {
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
											   "content-type:", "\n");
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
		std::string keep = get_header_value(_headers, "connection:", "\n");
		if (keep.empty()) {
			_headers += "Connection: close\r\n";
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

HttpCGIHandler::~HttpCGIHandler () {
	try {
		free_cgi_env();
		_log->log(LOG_DEBUG, CGI_NAME,
				  "CGI envs freed.");
	} catch (std::exception& e) {
		std::ostringstream details;
		details << "Error during CGI Destructor: " << e.what();
		_log->log(LOG_WARNING, CGI_NAME,
				  details.str());
	}
}

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
				_log->log(LOG_ERROR, CGI_NAME,
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
			_log->log(LOG_ERROR, CGI_NAME,
			          "execve function error.");
			exit(1);
		} else {
			close(cgi_in[0]);
			close(cgi_out[1]);
			if (!_request.body.empty()) {
				ssize_t written = write(cgi_in[1], _request.body.c_str(), _request.body.size());
				if (written == -1) {
					_log->log(LOG_ERROR, CGI_NAME,
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

void HttpCGIHandler::get_file_content(int pid, int (&fd)[2]) {
	char buffer[2048];
	std::string response;
	ssize_t bytes_read;
	_response_data.status = true;

	fcntl(fd[0], F_SETFL, O_NONBLOCK);
	struct pollfd pfd;
	pfd.fd = fd[0];
	pfd.events = POLLIN;
	_log->log(LOG_DEBUG, CGI_NAME,
			  "Reading CGI response.");
	while (true) {
		int poll_result = poll(&pfd, 1, CGI_TIMEOUT);
		if (pfd.revents & POLLHUP) {
			_log->log(LOG_DEBUG, CGI_NAME,
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

std::vector<char*> HttpCGIHandler::cgi_environment() {
	std::vector<std::string> env_vars;

	env_vars.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env_vars.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env_vars.push_back("HTTP_COOKIE=" + _request.cookie);
	env_vars.push_back("REQUEST_METHOD=" + method_enum_to_string(_request.method));
	env_vars.push_back("QUERY_STRING=" + _request.query);
	env_vars.push_back("CONTENT_TYPE=" + _request.content_type);
	env_vars.push_back("CONTENT_LENGTH=" + int_to_string((int)_request.content_length));
	env_vars.push_back("PATH_INFO=" + _request.path_info);
	env_vars.push_back("SCRIPT_NAME=" + _request.script);
	env_vars.push_back("SERVER_NAME=" + _client_data->get_server()->get_config().server_name);
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

void HttpCGIHandler::free_cgi_env() {
	for (size_t i = 0; i < _cgi_env.size() - 1; ++i) {
		free(_cgi_env[i]);
	}
}

bool HttpCGIHandler::send_response(const std::string &body, const std::string &path) {
	UNUSED(path);
	return(sender(body));
}
