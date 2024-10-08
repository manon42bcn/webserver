#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "SocketHandler.hpp"
#include "HttpRequestHandler.hpp"
#include "HttpResponseHandler.hpp"
#include <vector>
#include <poll.h>

/**
 * @brief Class to handle multiple instances of the server, to listen different ports
 *
 * Allows the creation of multiple instances to listen on different ports
 * handling petitions and responses.
 * Poll() function to multiplexer I/O.
 */
class ServerManager {
	private:
		std::vector<SocketHandler*> servers;
//		HttpRequestHandler request_handler;
//		HttpResponseHandler response_handler;
		std::vector<struct pollfd> poll_fds;

	public:
		ServerManager();
		~ServerManager();
		void add_server(int port);
		void run();

	private:
		void add_server_to_poll(int server_fd);
		void add_client_to_poll(int client_fd);
};

#endif
