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

bool HttpCGIHandler::handle_cgi() {
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
			_response_data.header = http_status_description(HTTP_OK);
		} else {
			int http_status = atoi(status.c_str());
			if (http_status_description((e_http_sts)http_status) != "No Info Associated") {
				_response_data.http_status = (e_http_sts)http_status;
			} else {
				turn_off_sanity(HTTP_BAD_GATEWAY,
								"Non valid HTTP status provided by CGI.");
				send_error_response();
				return (false);
			}
		}

		_response_data.content = _response_data.content.substr(header_pos + 1);
		return (true);
	} else {
		turn_off_sanity(HTTP_BAD_GATEWAY,
						"CGI does not include a response.");
		send_error_response();
	}

	return (false);
}

bool HttpCGIHandler::cgi_execute() {
	int cgi_in[2];
	int cgi_out[2];

	if (pipe(cgi_in) == -1 || pipe(cgi_out) == -1) {
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
						"Error building pipes to CGI handle.");
		return false;
	}

	_cgi_env = cgi_environment();
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
		dup2(cgi_in[0], STDIN_FILENO);
		dup2(cgi_out[1], STDOUT_FILENO);
		close(cgi_in[1]);
		close(cgi_out[0]);
		close(cgi_in[0]);
		close(cgi_out[1]);

		std::string cgi_path = _request.normalized_path + _request.script;
		char* const argv[] = { const_cast<char*>(cgi_path.c_str()), NULL };
		execve(cgi_path.c_str(), argv, _cgi_env.data());

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
		bool reading = read_from_cgi(pid, cgi_out);
		close(cgi_in[0]);
		free_cgi_env();
		if (!reading) {
			send_error_response();
		}
		return (true);
	}
}

bool HttpCGIHandler::read_from_cgi(int pid, int (&fd)[2]) {
	char buffer[2048];
	std::string response;
	ssize_t bytes_read;
	bool active = true;

	fcntl(fd[0], F_SETFL, O_NONBLOCK);
	struct pollfd pfd;
	pfd.fd = fd[0];
	pfd.events = POLLIN;
	_log->log(LOG_DEBUG, RSP_NAME,
			  "Reading CGI response.");
//	TODO: Improve WAIT to get exit status
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
			active = false;
			break;
		} else if (poll_result == -1) {
			turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
							"Poll error on CGI pipe.");
			active = false;
			break;
		}
		if (pfd.revents & POLLIN) {
			bytes_read = read(fd[0], buffer, sizeof(buffer) - 1);

			if (bytes_read > 0) {
				buffer[bytes_read] = '\0';
				response += buffer;
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
	if (!active) {
		kill(pid, SIGKILL);
		waitpid(pid, NULL, WNOHANG);
	}
	waitpid(pid, NULL, 0);

//	_response_data.content.resize(response.length());
	_response_data.content = response;
	return (active);
}


std::vector<char*> HttpCGIHandler::cgi_environment() {
	std::vector<std::string> env_vars;

	// Variables estándar de CGI
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

	// Variables adicionales de CGI
//	env_vars.push_back("REMOTE_ADDR=" + _client_data->get_client_ip());
//	env_vars.push_back("HTTP_USER_AGENT=" + _request.user_agent);

	// Convertir std::vector<std::string> a std::vector<char*>
	std::vector<char*> env_ptrs;
	for (size_t i = 0; i < env_vars.size(); ++i) {
		// Usamos strdup para crear una copia mutable de cada cadena
		env_ptrs.push_back(strdup(env_vars[i].c_str()));
	}
	env_ptrs.push_back(NULL);  // Finalización del entorno con NULL
//	for (size_t i = 0; i < env_ptrs.size(); i++){
//		_log->log(LOG_DEBUG, RSP_NAME, env_ptrs[i]);
//	}
	return env_ptrs;
}

void HttpCGIHandler::free_cgi_env() {
	for (size_t i = 0; i < _cgi_env.size() - 1; ++i) {
		free(_cgi_env[i]);  // Libera cada cadena duplicada con strdup
	}
}
