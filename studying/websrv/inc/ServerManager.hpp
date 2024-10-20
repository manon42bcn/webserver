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
#include "ClientData.hpp"
#include "webserver.hpp"
#include "Logger.hpp"

#define SM_NAME "ServerManager"

/**
 * @brief Manages multiple server sockets and handles incoming connections.
 */
class ServerManager {
	private:
		std::vector<struct pollfd> 	    _poll_fds;
		std::vector<SocketHandler*>     _servers;
	    std::vector<ClientData> 	    _clients;
		const Logger*			        _log;
public:
		ServerManager(const std::vector<ServerConfig>& configs, const Logger* logger);
	    ~ServerManager();
	    void add_server(int port, const ServerConfig& config);
		void run();
	    void new_client(SocketHandler* server);
		bool add_server_to_poll(int server_fd);
	    void remove_client_from_poll(int fd);
	    bool process_request(int poll_fd);
};

#endif
