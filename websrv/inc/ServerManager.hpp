/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/10 03:00:53 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _SERVERMANAGER_HPP_
#define _SERVERMANAGER_HPP_

#include <vector>
#include <poll.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include "http_enum_codes.hpp"
#include "SocketHandler.hpp"
#include "HttpRequestHandler.hpp"
#include "WebserverCache.hpp"
#include "ClientData.hpp"
#include "webserver.hpp"
#include "Logger.hpp"

#define SM_NAME "ServerManager"
typedef std::map<int, ClientData*>::iterator t_client_it;

/**
 * @class ServerManager
 * @brief Manages the operation of web server instances, client connections, and network events.
 *
 * The `ServerManager` class is responsible for:
 * - Initializing and managing multiple server instances (`SocketHandler` objects) based on the provided configuration.
 * - Maintaining active client connections and polling their events.
 * - Handling critical server functions, such as timeouts, connection cleanup, and error handling.
 *
 * This class uses a `poll`-based event loop in the `run` method to handle incoming connections and requests.
 * It provides a shutdown mechanism in case of unrecoverable errors and is equipped with logging for server status tracking.
 */
class ServerManager {
	private:
		std::vector<struct pollfd> 	    _poll_fds;
		std::vector<SocketHandler*>     _servers;
	    std::map<int, ClientData*>      _clients;
		const Logger*			        _log;
		WebServerCache*					_cache;
		bool                            _active;
		bool                            _healthy;

	void add_server(int port, ServerConfig& config);
	bool add_server_to_poll(int server_fd);
	void cleanup_invalid_fds();
	void timeout_clients();
	void new_client(SocketHandler* server);
	bool process_request(size_t& poll_fd_index);
	void remove_client_from_poll(t_client_it client_data,
	                             size_t& poll_index);
	bool turn_off_sanity(const std::string& detail);
	void clear_clients();
	void clear_servers();
	void clear_poll();
public:
		ServerManager(std::vector<ServerConfig>& configs,
						  const Logger* logger,
						  WebServerCache* cache);
		~ServerManager();
		void run();
		void turn_off_server();
};

#endif