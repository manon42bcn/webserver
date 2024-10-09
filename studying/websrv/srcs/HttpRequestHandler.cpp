#include "HttpRequestHandler.hpp"
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>

/**
 * @brief Read the request from the socket.
 *
 * @param client_socket Client FD
 * @return std::string Full request string format.
 */
std::string HttpRequestHandler::read_http_request(int client_socket) {
	char buffer[1024];
	std::string request;
	int valread;

	while ((valread = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
		buffer[valread] = '\0';
		request += buffer;
		if (request.find("\r\n\r\n") != std::string::npos) {
			break;
		}
	}

	return (request);
}

/**
 * @brief Parse requested route
 *
 * @param request Request string format
 * @return std::string Requested route
 */
std::string HttpRequestHandler::parse_requested_path(const std::string& request) {
	size_t pos = request.find(" ");
	if (pos != std::string::npos) {
		size_t end_pos = request.find(" ", pos + 1);
		if (end_pos != std::string::npos) {
			return request.substr(pos + 1, end_pos - pos - 1);
		}
	}
	return "/";
}

/**
 * @brief HTTP Request handler
 *
 * @param client_socket Client FD.
 * @param config Server config struct
 */
void HttpRequestHandler::handle_request(int client_socket, const ServerConfig& config) {
	std::string request = read_http_request(client_socket);
	std::string requested_path = parse_requested_path(request);

	if (requested_path == "/") {
		for (size_t i = 0; i < config.default_pages.size(); ++i) {
			std::string full_path = config.document_root + "/" + config.default_pages[i];
			std::ifstream file(full_path.c_str(), std::ios::binary);
			if (file.is_open()) {
				std::stringstream file_content;
				file_content << file.rdbuf();
				std::string content = file_content.str();

				std::string response = "HTTP/1.1 200 OK\r\n";
				response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
				response += "Content-Type: text/html\r\n";
				response += "\r\n";
				response += content;
				send(client_socket, response.c_str(), response.length(), 0);
				file.close();
				return;
			}
		}
	}

	if (requested_path.back() == '/') {
		for (size_t i = 0; i < config.default_pages.size(); ++i) {
			std::string full_path = config.document_root + requested_path + config.default_pages[i];
			std::ifstream file(full_path.c_str(), std::ios::binary);
			if (file.is_open()) {
				std::stringstream file_content;
				file_content << file.rdbuf();
				std::string content = file_content.str();

				std::string response = "HTTP/1.1 200 OK\r\n";
				response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
				response += "Content-Type: text/html\r\n";
				response += "\r\n";
				response += content;
				send(client_socket, response.c_str(), response.length(), 0);
				file.close();
				return;
			}
		}
	}

	std::string full_path = config.document_root + requested_path;
	std::ifstream file(full_path.c_str(), std::ios::binary);

	if (file.is_open()) {
		std::stringstream file_content;
		file_content << file.rdbuf();
		std::string content = file_content.str();

		std::string response = "HTTP/1.1 200 OK\r\n";
		response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
		response += "Content-Type: text/html\r\n";
		response += "\r\n";
		response += content;

		send(client_socket, response.c_str(), response.length(), 0);
	} else {

		std::string error_response = config.error_pages.count(404) ? config.error_pages.at(404) : "404 - File Not Found";
		std::string response = "HTTP/1.1 404 Not Found\r\nContent-Length: " + std::to_string(error_response.length()) + "\r\n\r\n";
		response += error_response;
		send(client_socket, response.c_str(), response.length(), 0);
	}

	file.close();
}
