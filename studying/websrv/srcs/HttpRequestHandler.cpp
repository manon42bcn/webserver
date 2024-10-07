#include "HttpRequestHandler.hpp"
#include <unistd.h>
#include <cstring>
#include <iostream>

/**
 * @brief Read request from a socket as string.
 *
 * @param client_socket Client Socket FD.
 * @return std::string Http request as string.
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
 * @brief Parse HTTP headers from a request.
 *
 * @param request HTTP request as string
 * @return std::string Headers of the request.
 */
std::string HttpRequestHandler::parse_headers(const std::string& request) {
	size_t header_end = request.find("\r\n\r\n");
	if (header_end != std::string::npos) {
		return request.substr(0, header_end);
	}
	return ("");
}

/**
 * @brief Handler HTTP Get requests.
 *
 * @param client_socket Client Socket FD.
 */
void HttpRequestHandler::handle_get(int client_socket) {
	std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, GET!";
	send(client_socket, response.c_str(), response.length(), 0);
}

/**
 * @brief Handler HTTP POST requests.
 *
 * @param client_socket Client Socket FD.
 */
void HttpRequestHandler::handle_post(int client_socket) {
	std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 14\r\n\r\nHello, POST!";
	send(client_socket, response.c_str(), response.length(), 0);
}

/**
 * @brief Request handler
 *
 * @param client_socket Client Socket FD.
 */
void HttpRequestHandler::handle_request(int client_socket) {
	std::string request = read_http_request(client_socket);
	std::string method = request.substr(0, request.find(" "));

	if (method == "GET") {
		handle_get(client_socket);
	} else if (method == "POST") {
		handle_post(client_socket);
	} else {
		std::string response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 23\r\n\r\nMethod Not Allowed";
		send(client_socket, response.c_str(), response.length(), 0);
	}
}
