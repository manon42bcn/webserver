#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <vector>
#include <poll.h>
#include "SocketHandler.hpp"
#include "HttpRequestHandler.hpp"
#include "webserver.hpp"

/**
 * @brief Manages multiple server sockets and handles incoming connections.
 */
class ServerManager {
private:
	std::vector<struct pollfd> poll_fds;           ///< Vector of file descriptors for poll().
	std::vector<SocketHandler*> servers;           ///< Vector of server socket handlers (one per port).
	std::vector<const ServerConfig> client_configs; ///< Vector to store configurations associated with clients.

public:
	ServerManager(std::vector<ServerConfig> configs);
	void run();
	void add_server(int port, ServerConfig config);
	void add_client_to_poll(int client_fd);
	void add_server_to_poll(int server_fd);
};

#endif
