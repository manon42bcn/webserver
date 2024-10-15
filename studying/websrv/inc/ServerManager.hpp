/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/14 14:04:11 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <vector>
#include <poll.h>
#include "SocketHandler.hpp"
#include "HttpRequestHandler.hpp"
#include "webserver.hpp"

/**
 * @brief Structure to associate a client with the server that accepted the connection.
 */
struct ClientInfo {
	SocketHandler*  server;
	struct pollfd   client_fd;
};

/**
 * @brief Manages multiple server sockets and handles incoming connections.
 */
class ServerManager {
	private:
		std::vector<struct pollfd> _poll_fds;  ///< Vector of file descriptors for poll()
		std::vector<SocketHandler*> _servers;  ///< Vector of server socket handlers (one per port)
		std::vector<ClientInfo> _clients;      ///< Vector of clients with their associated servers

public:
		ServerManager(const std::vector<ServerConfig>& configs);
		void run();
		void add_server(int port, const ServerConfig& config);
		void add_client_to_poll(int client_fd);
		void add_server_to_poll(int server_fd);
};

#endif
