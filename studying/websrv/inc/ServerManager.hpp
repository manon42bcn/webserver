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
	    std::map<int, ClientData*>      _clients_map;
		const Logger*			        _log;
		bool                            _active;

public:
		typedef std::map<int, ClientData*>::iterator t_client_it;
		ServerManager(std::vector<ServerConfig>& configs, const Logger* logger);
	    ~ServerManager();
	    void add_server(int port, ServerConfig& config);
		void run();
	    void new_client(SocketHandler* server);
		bool add_server_to_poll(int server_fd);
	    void remove_client_from_poll(t_client_it client_data, size_t poll_index);
	    bool process_request(size_t poll_fd_index);
	    void turn_off_server();
	    class ServerBuildError : public std::exception {
			public:
			    virtual const char *what() const throw();
	    };
};

#endif
