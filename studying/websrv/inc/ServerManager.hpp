/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/18 13:51:35 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __SERVERMANAGER_HPP__
#define __SERVERMANAGER_HPP__

#include <vector>
#include <poll.h>
#include "http_enum_codes.hpp"
#include "SocketHandler.hpp"
#include "HttpRequestHandler.hpp"
#include "webserver.hpp"
#include "Logger.hpp"


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
		std::vector<struct pollfd> 	_poll_fds;
		std::vector<SocketHandler*> _servers;
		std::vector<ClientInfo> 	_clients;
		const std::string			_module;
		const Logger*				_log;
public:
		ServerManager(const std::vector<ServerConfig>& configs, const Logger* logger);
		void add_server(int port, const ServerConfig& config);
		void run();
		void add_client_to_poll(int client_fd);
		void add_server_to_poll(int server_fd);
};

#endif
