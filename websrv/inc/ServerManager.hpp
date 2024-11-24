/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/24 22:36:49 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _SERVERMANAGER_HPP_
#define _SERVERMANAGER_HPP_

#include <vector>
#include <poll.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <sys/time.h>
#include "http_enum_codes.hpp"
#include "SocketHandler.hpp"
#include "HttpRequestHandler.hpp"
#include "HttpResponseHandler.hpp"
#include "HttpCGIHandler.hpp"
#include "HttpRangeHandler.hpp"
#include "HttpMultipartHandler.hpp"
#include "HttpAutoIndex.hpp"
#include "WebserverCache.hpp"
#include "ClientData.hpp"
#include "webserver.hpp"
#include "Logger.hpp"
//defined in microsecs
#define CLIENT_LIFECYCLE 10000000
#define SM_NAME "ServerManager"
typedef std::map<int, ClientData*>::iterator t_client_it;
typedef std::map<int, time_t>::iterator t_fd_timestamp;
typedef std::map<time_t, int>::iterator t_timestamp_fd;

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
		std::map<int, size_t>           _poll_index;
		std::map<int, SocketHandler*>   _servers_map;
		std::map<int, int>              _active_ports;
	    std::map<int, ClientData*>      _clients;
		std::map<time_t, int>           _timeout_index;
		std::map<int, time_t>           _index_timeout;
		const Logger*			        _log;
		bool                            _active;
		bool                            _healthy;

	bool add_server(int port, ServerConfig& config);
	void build_servers(std::vector<ServerConfig>& configs);
	bool add_server_to_poll(int server_fd);
	void cleanup_invalid_fds();
	void timeout_clients();
	bool new_client(SocketHandler* server);
	bool process_request(size_t& poll_fd_index);
	bool process_response(size_t& poll_fd_index, t_client_it client);
	void remove_client_from_poll(t_client_it client_data);
	bool turn_off_sanity(const std::string& detail);
	void clear_clients();
	void clear_servers();
	void clear_poll();
	static time_t timeout_timestamp();
public:
		ServerManager(std::vector<ServerConfig>& configs,
					  const Logger* logger);
		~ServerManager();
		void run();
		void turn_off_server();
};

#endif
