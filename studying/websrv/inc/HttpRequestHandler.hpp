#ifndef HTTPREQUESTHANDLER_HPP
#define HTTPREQUESTHANDLER_HPP

#include "webserver.hpp"
#include <string>

/**
 * @brief Class to encapsulate HTTP request handler functionality.
 */
class HttpRequestHandler {
public:
	void handle_request(int client_socket, const ServerConfig& config);

private:
	std::string parse_requested_path(const std::string& request);
	std::string read_http_request(int client_socket);
};

#endif
