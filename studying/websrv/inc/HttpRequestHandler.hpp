#ifndef HTTPREQUESTHANDLER_HPP
#define HTTPREQUESTHANDLER_HPP

#include <string>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>

/**
 * @brief Class to handle HTTP requests.
 *
 * HttpRequestHandler encapsulates methods to handle HTTP requests froma socket.
 * Parse payload an different HTTP methods (GET, POST)
 */
class HttpRequestHandler {
public:
	void handle_request(int client_socket);

private:
	std::string parse_headers(const std::string& request);
	void handle_get(int client_socket);
	void handle_post(int client_socket);
	std::string read_http_request(int client_socket);
};

#endif
