/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/18 15:11:53 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerManager.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>

/**
 * @brief Constructor of ServerManager that receives server configurations.
 *
 * @param configs Vector with the configurations of the servers.
 */
ServerManager::ServerManager(const std::vector<ServerConfig>& configs, Logger* logger):
							_module("ServerManager"),
							_log(logger) {
	_poll_fds.reserve(100);
	for (size_t i = 0; i < configs.size(); ++i) {
		add_server(configs[i].port, configs[i]);
	}
	_log->log(LOG_DEBUG, _module, "instance init and ready.");
}

/**
 * @brief Adds a new server with the specified port.
 *
 * @param port Port number where the server will listen.
 * @param config Configuration of the server.
 */
void ServerManager::add_server(int port, const ServerConfig& config) {
	SocketHandler* server = new SocketHandler(port, config);
	_servers.push_back(server);
	_log->log(LOG_DEBUG, _module, "SocketHandler instance created and append to _servers.");
	add_server_to_poll(server->get_socket_fd());
}

/**
 * @brief Adds a server socket to the poll set to monitor its activity.
 *
 * @param server_fd File descriptor of the server socket.
 */
void ServerManager::add_server_to_poll(int server_fd) {
	struct pollfd pfd;
	pfd.fd = server_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_poll_fds.push_back(pfd);
	_log->log(LOG_DEBUG, _module, "server fd add to _polls_fds.");
}

/**
 * @brief Adds a new client to the poll set and associates it with a server configuration.
 *
 * @param client_fd File descriptor of the client.
 * @param config Server configuration associated with this client.
 */
void ServerManager::add_client_to_poll(int client_fd) {
	struct pollfd pfd;
	pfd.fd = client_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_poll_fds.push_back(pfd);
	_log->log(LOG_DEBUG, _module, "client fd add to _polls_fds.");
}

/**
 * @brief Main event loop for handling incoming connections and client requests.
 */
void ServerManager::run() {
	_log->log(LOG_DEBUG, _module, "Event loop started.");
	while (true) {
		int poll_count = poll(&_poll_fds[0], _poll_fds.size(), -1);

		if (poll_count < 0) {
			_log->log(LOG_ERROR, _module, "Error in poll()");
			std::cerr << "Error in poll()" << std::endl;
			exit(EXIT_FAILURE);
		}

		for (size_t i = 0; i < _poll_fds.size(); ++i) {
			if (_poll_fds[i].revents & POLLIN) {
				bool is_server = false;
				SocketHandler* server = NULL;

				// Check if the descriptor corresponds to a server socket
				for (size_t s = 0; s < _servers.size(); ++s) {
					if (_poll_fds[i].fd == _servers[s]->get_socket_fd()) {
						_log->log(LOG_DEBUG, _module, "fd belongs to a server.");
						is_server = true;
						server = _servers[s];
						break;
					}
				}

				if (is_server) {
					// Accept a new connection
					int new_client_fd = server->accept_connection();
					if (new_client_fd > 0) {
						ClientInfo client_info;
						client_info.server = server;
						client_info.client_fd.fd = new_client_fd;
						client_info.client_fd.events = POLLIN;

						// Add client info to clients and poll list
						_clients.push_back(client_info);
						_poll_fds.push_back(client_info.client_fd);
						_log->log(LOG_DEBUG, _module,
								  "New connection accepted on port " + int_to_string(server->get_socket_fd()));
					}
				} else {
					// Handle client request
					ClientInfo* client_info = NULL;

					// Find the corresponding client in the clients vector
					for (size_t c = 0; c < _clients.size(); ++c) {
						if (_poll_fds[i].fd == _clients[c].client_fd.fd) {
							client_info = &_clients[c];
							break;
						}
					}

					if (client_info != NULL) {
						HttpRequestHandler request_handler(_poll_fds[i].fd, client_info->server->get_config());
//						request_handler.handle_request(_poll_fds[i].fd, client_info->server->get_config());

						// Remove the client info from the _clients vector BEFORE closing the connection
						for (size_t c = 0; c < _clients.size(); ++c) {
							if (_clients[c].client_fd.fd == _poll_fds[i].fd) {
								_clients.erase(_clients.begin() + (int)c);
								_log->log(LOG_DEBUG, _module, "client remove from _client vector.");
								break;
							}
						}
					}

					// Close the client connection
					close(_poll_fds[i].fd);
					_log->log(LOG_DEBUG, _module, "close connection: " + int_to_string((int)i));
					// Remove the client descriptor from _poll_fds
					_poll_fds.erase(_poll_fds.begin() + (int)i);
					_log->log(LOG_DEBUG, _module, "fd remove from _polls_fds vector: " + int_to_string((int)i));
					--i;  // Adjust index to check the new descriptor in this position
				}
			}
		}
	}
}
