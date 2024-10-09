#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP
#include "webserver.hpp"
#include <vector>
#include <poll.h>
#include <map>
#include "HttpRequestHandler.hpp"
#include "HttpResponseHandler.hpp"
#include "SocketHandler.hpp"

/**
 * @brief Class to handle multiple instances of the server, to listen different ports
 *
 * Allows the creation of multiple instances to listen on different ports
 * handling petitions and responses.
 * Poll() function to multiplexer I/O.
 */
class ServerManager {
private:
		std::vector<SocketHandler*> servers;           ///< Vector de punteros a SocketHandler.
		std::vector<ServerConfig> server_configs;      ///< Vector que contiene la configuración de los servidores.
		std::vector<struct pollfd> poll_fds;

public:
		ServerManager(const std::vector<ServerConfig>& configs);
//		~ServerManager();
		void add_server(int port, const ServerConfig& config);
		void run();

	private:
		void add_server_to_poll(int server_fd);
		void add_client_to_poll(int client_fd);
};

#endif
