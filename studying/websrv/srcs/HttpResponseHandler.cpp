/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseHandler.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/04 10:34:27 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponseHandler.hpp"

HttpResponseHandler::HttpResponseHandler(const LocationConfig *location,
                                         const Logger *log,
										 ClientData* client_data,
										 s_request& request,
										 int fd):
		_fd(fd),
        _location(location),
		_log(log),
		_client_data(client_data),
		_request(request) {
	if (_log == NULL)
		throw Logger::NoLoggerPointer();
	_log->log(LOG_DEBUG, RSP_NAME,
	          "HttpResponseHandler init.");
}

bool HttpResponseHandler::handle_request() {

	if (!_request.sanity) {
		send_error_response();
		return (false);
	}
	if (_request.cgi) {
		handle_cgi();
		if (!_request.sanity) {
			send_error_response();
		}
		sender(_response, _request.path);
	}
	switch (_request.method) {
		case METHOD_GET:
			_log->log(LOG_DEBUG, RSP_NAME,
			          "Handle GET request.");
			return (handle_get());
		case METHOD_POST:
			_log->log(LOG_DEBUG, RSP_NAME,
			          "Handle POST request.");
			return 	(handle_post());
			break;
		case METHOD_DELETE:
			_log->log(LOG_DEBUG, RSP_NAME,
			          "Handle DELETE request.");
			handle_delete();
			break;
		default:
			turn_off_sanity(HTTP_NOT_IMPLEMENTED,
			                "Method not allowed.");
			send_error_response();
	}
	return (true);
}

bool HttpResponseHandler::handle_get() {
	if (_request.status != HTTP_OK || _request.access < ACCESS_READ) {
		send_error_response();
		return (false);
	}
	s_content content = get_file_content(_request.normalized_path);
	if (content.status) {
		_log->log(LOG_DEBUG, RSP_NAME,
		          "File content will be sent.");
		return (sender(content.content, _request.normalized_path));
	} else {
		_log->log(LOG_ERROR, RSP_NAME,
		          "Get will send a error due to content load fails.");
		return (send_error_response());
	}
}

s_content HttpResponseHandler::get_file_content(std::string& path) {
	std::string content;
	// Check if path is empty. To avoid further unnecessary errors
	if (path.empty())
		return (s_content(false, ""));

	try {
		std::ifstream file(path.c_str(), std::ios::binary);

		if (!file) {
			_log->log(LOG_ERROR, RSP_NAME,
			          "Failed to open file: " + path);
			_request.status = HTTP_FORBIDDEN;
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
			_request.status = HTTP_INTERNAL_SERVER_ERROR;
			return (s_content(false, ""));
		}
		file.close();
	} catch (const std::ios_base::failure& e) {
		_log->log(LOG_ERROR, RSP_NAME,
		          "I/O failure: " + std::string(e.what()));
		_request.status = HTTP_INTERNAL_SERVER_ERROR;
		return (s_content(false, ""));
	} catch (const std::exception& e) {
		_log->log(LOG_ERROR, RSP_NAME,
		          "Exception: " + std::string(e.what()));
		_request.status = HTTP_INTERNAL_SERVER_ERROR;
		return (s_content(false, ""));
	}
	_log->log(LOG_DEBUG, RSP_NAME,
	          "File content read OK.");
	return (s_content(true, content));
}

std::string HttpResponseHandler::header(int code, size_t content_size, std::string mime) {
	std::ostringstream header;
	header << "HTTP/1.1 " << code << " " << http_status_description((e_http_sts)code) << "\r\n"
	       << "Content-Length: " << content_size << "\r\n"
	       << "Content-Type: " <<  mime << "\r\n"
	       << "Connection: close\r\n"
	       << "\r\n";

	return (header.str());
}


std::string HttpResponseHandler::default_plain_error() {
	std::ostringstream content;
	content << "<!DOCTYPE html>\n"
	        << "<html>\n<head>\n"
	        << "<title>Webserver - Error</title>\n"
	        << "</head>\n<body>\n"
	        << "<h1>Something went wrong...</h1>\n"
	        << "<h2>" << int_to_string(_request.status) << " - "
	        << http_status_description(_request.status) << "</h2>\n"
	        << "</body>\n</html>\n";
	_log->log(LOG_DEBUG, RSP_NAME, "Build default error page.");
	return (content.str());
}

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
			it = error_pages->find(_request.status);
		}
		if (!error_pages->empty() || it != error_pages->end()) {
			s_content file_content = get_file_content(error_file);
			if (!file_content.status) {
				_log->log(LOG_DEBUG, RSP_NAME,
				          "Custom error page cannot be load. http status: " + int_to_string(_request.status));
				content = default_plain_error();
			} else {
				_log->log(LOG_DEBUG, RSP_NAME, "Custom error page found.");
				content = file_content.content;
				file_path = error_file;
				if (_location->loc_error_mode == TEMPLATE)
				{
					content = replace_template(content, "{error_code}",
					                           int_to_string(_request.status));
					content = replace_template(content, "{error_detail}",
					                           http_status_description(_request.status));
					_log->log(LOG_DEBUG, RSP_NAME, "Template update to send.");
				}
			}
		} else {
			_log->log(LOG_DEBUG, RSP_NAME,
			          "Custom error page was not found. Error: " + int_to_string(_request.status));
		}
	}
	return (sender(content, file_path));
}

bool HttpResponseHandler::sender(const std::string& body, const std::string& path) {
	try {
		std::string mime_type;
		if (_response_type.empty()) {
			mime_type = get_mime_type(path);
		} else {
			mime_type = _response_type;
		}
		std::string response = header(_request.status, body.length(), mime_type);
		response += body;
		int total_sent = 0;
		int to_send = (int)response.length();
		while (total_sent < to_send) {
			int sent_bytes = (int)send(_fd, response.c_str() + total_sent, to_send - total_sent, 0);
			// TODO: videos can overlog errors... check for a reasonable fix
			if (sent_bytes == -1) {
				_log->log(LOG_DEBUG, RSP_NAME,
				          "Error sending the response.");
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

void HttpResponseHandler::turn_off_sanity(e_http_sts status, std::string detail) {
	_log->log(LOG_ERROR, RSP_NAME, detail);
	_request.sanity = false;
	_request.status = status;
}

void HttpResponseHandler::get_post_content(){
	if (!_request.sanity) {
		return;
	}
	if (_request.content_length == 0) {
		turn_off_sanity(HTTP_LENGTH_REQUIRED,
		                "Content-length required at POST request.");
		return;
	}
	if (_request.content_type.empty()) {
		turn_off_sanity(HTTP_BAD_REQUEST,
		                "Content-Type required at POST request.");
		return;
	}
	if (_request.body.empty()) {
		turn_off_sanity(HTTP_BAD_REQUEST,
		                "No body request required at POST request.");
		return;
	} else {
		if (_request.body.length() != _request.content_length) {
			turn_off_sanity(HTTP_BAD_REQUEST,
			                "Received content-length and content size does not match.");
			return ;
		}
	}
	if (!_request.boundary.empty()) {
		parse_multipart_data();
	} else {
		_multi_content.push_back(s_multi_part(_request.normalized_path, CT_FILE, _request.body));
	}
}

void HttpResponseHandler::parse_multipart_data() {
	size_t part_start = _request.body.find(_request.boundary);
	while (part_start != std::string::npos) {
		part_start += _request.boundary.length() + 2;
		size_t headers_end = _request.body.find("\r\n\r\n", part_start);
		if (headers_end != std::string::npos) {
			_log->log(LOG_DEBUG, RSP_NAME,
					  "Looking for Post multi-part related data.");
			std::string content_info = _request.body.substr(part_start, headers_end - part_start);
			size_t file_data_start = headers_end + 4;
			size_t next_boundary = _request.body.find(_request.boundary, file_data_start);
			std::string content_data = _request.body.substr(file_data_start, next_boundary - file_data_start);
			if (content_info.empty() || content_data.empty()) {
				turn_off_sanity(HTTP_BAD_REQUEST,
								"Multi-part body request is incomplete.");
				return ;
			}
			std::string content_disposition = get_header_value(content_info, "content-disposition:", "\r\n");
			s_multi_part part_data(get_header_value(content_info, "content-disposition:", ";"),
								   trim(get_header_value(content_disposition, "name", ";"), "\""),
								   trim(get_header_value(content_disposition, "filename", ";"), "\""),
								   get_header_value(content_info, "content-type:", "\r\n"),
								   content_data);
			if (part_data.filename.empty()) {
				part_data.data_type = CT_UNKNOWN;
			}
			_multi_content.push_back(part_data);
			part_start = next_boundary;
		} else {
			_log->log(LOG_DEBUG, RSP_NAME,
					  "End of multi-part body request.");
			break;
		}
	}
}

void HttpResponseHandler::validate_payload() {
	if (!_request.sanity) {
		return;
	}
	for (size_t i = 0; i < _multi_content.size(); i++) {
		if (!_multi_content[i].filename.empty() && black_list_extension(_multi_content[i].filename)) {
			turn_off_sanity(HTTP_UNSUPPORTED_MEDIA_TYPE,
								"File extension is on black-list.");
			return ;
		}
	}
}

// Validation of access privileges will be done before.
bool HttpResponseHandler::handle_post() {
	if (_location->loc_access < ACCESS_WRITE) {
		turn_off_sanity(HTTP_FORBIDDEN,
						"No post data available.");
	}
	get_post_content();
	validate_payload();
	_client_data->chronos_reset();
	if (!_request.sanity) {
		send_error_response();
	}

	std::string save_path;
	for (size_t i = 0; i < _multi_content.size(); i++) {
		if (_multi_content[i].data_type == CT_FILE) {
			save_path = _multi_content[1].filename;
			if (!_request.boundary.empty()) {
				save_path = _request.normalized_path + _multi_content[i].filename;
			}
			_log->log(LOG_DEBUG, RSP_NAME, "path: " + save_path);
			std::ifstream check_file(save_path.c_str());
			if (check_file.good()) {
				turn_off_sanity(HTTP_CONFLICT,
				                "File already exists and cannot be overwritten.");
				check_file.close();
				return (send_error_response());
			}
			std::ofstream file(save_path.c_str());
			if (!file.is_open()) {
				turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
								"Unable to open file to write.");
				return (send_error_response());
			}
			file << _multi_content[i].data;
			file.close();
			_request.status = HTTP_CREATED;
			_log->log(LOG_DEBUG, RSP_NAME,
					  "File Data Recieved and saved.");
		}
	}
	sender("Created", _request.normalized_path);
	return (true);
}

bool HttpResponseHandler::handle_delete() {
	if (_location->loc_access < ACCESS_DELETE) {
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
	_log->log(LOG_DEBUG, RSP_NAME,
	          "Resource deleted successfully: " + delete_path);
	sender("Resource Deleted.", delete_path);
	return (true);
}

size_t end_of_header_system(std::string& header)
{
	size_t  pos = header.find("\r\n\r\n");
	if (pos == std::string::npos) {
		pos = header.find("\n\n");
	}
	return (pos);
}

bool HttpResponseHandler::handle_cgi() {
	cgi_execute();
	if (!_response.empty()) {
		size_t header_pos = end_of_header_system(_response);
		if (header_pos == std::string::npos) {
			turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
							"CGI Response does not include a valid header.");
			send_error_response();
			return (false);
		}
		_response_type = get_header_value(_response, "content-type:", "\n");
		if (_response_type.empty()){
			turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
							"Content-Type not present at CGI response.");
			send_error_response();
			return (false);
		}
		std::string status = get_header_value(_response, "Status:", " ");
		if (status.empty()) {
			status = "200";
		}
		if (is_valid_size_t(status)) {
			_request.status = (e_http_sts)str_to_size_t(status);
		} else {
			turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
							"Malformed Status included at CGI response.");
			send_error_response();
			return (false);
		}
		std::string header = _response.substr(0,header_pos);
		std::string response = _response.substr(header_pos + 1);
		_response_type = get_header_value(_response, "content-type:", "\n");
		_response = _response.substr(_response.find('\n') + 1);
		return (true);
	} else {
		turn_off_sanity(HTTP_BAD_GATEWAY,
						"CGI does not include a response.");
		send_error_response();
	}

	return (false);
}

bool HttpResponseHandler::cgi_execute() {
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

bool HttpResponseHandler::read_from_cgi(int pid, int (&fd)[2]) {
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
	while (true) {
		int poll_result = poll(&pfd, 1, CGI_TIMEOUT);
		if (pfd.revents & POLLHUP) {
			_log->log(LOG_DEBUG, RSP_NAME,
					  "CGI pipe closed by writer.");
			break;
		}
		if (poll_result == 0) {
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

	_response = response;
	return (active);
}
//bool HttpResponseHandler::read_from_cgi(int pid, int (&fd)[2]) {
//
//	char buffer[1024];
//	std::string response;
//	ssize_t bytes_read;
//	_client_data->chronos_reset();
//	bool active = true;
//	while ((bytes_read = read(fd[0], buffer, sizeof(buffer) - 1)) > 0) {
//		if (!(active = _client_data->chronos())) {
//			break;
//		}
//		buffer[bytes_read] = '\0';
//		response += buffer;
//	}
//	close(fd[0]);
//	if (active){
////		TODO: Verificar si vale la pena obtener el status de salida.
//		waitpid(pid, NULL, 0);
//	} else {
//		kill(pid, SIGKILL);
//		turn_off_sanity(HTTP_GATEWAY_TIMEOUT,
//		                "CGI Response timed out.");
//	}
//	if (bytes_read == -1) {
//		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
//		                "Error reading CGI response.");
//	}
//	_log->log(LOG_DEBUG, RSP_NAME,
//			  "LLegamos aqui...");
//	_response = response;
//	return (active);
//}

std::vector<char*> HttpResponseHandler::cgi_environment() {
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

void HttpResponseHandler::free_cgi_env() {
	for (size_t i = 0; i < _cgi_env.size() - 1; ++i) {
		free(_cgi_env[i]);  // Libera cada cadena duplicada con strdup
	}
}
