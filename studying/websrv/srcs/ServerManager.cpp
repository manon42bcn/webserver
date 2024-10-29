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


ServerManager::ServerManager(std::vector<ServerConfig>& configs, const Logger* logger):
							_log(logger) {
	if (_log == NULL) {
		throw Logger::NoLoggerPointer();
	}
	_poll_fds.reserve(100);
	try {
		for (size_t i = 0; i < configs.size(); ++i) {
			add_server(configs[i].port, configs[i]);
		}
	} catch (const SocketHandler::SocketCreationError &e) {
		_log->log(LOG_ERROR, SM_NAME, e.what());
		throw ServerManager::ServerBuildError();
	}
	_active = true;
	_log->log(LOG_DEBUG, SM_NAME, "instance init and ready.");
}

ServerManager::~ServerManager() {
	if (!_clients_map.empty()) {
		_log->log(LOG_DEBUG, SM_NAME,
		          "Cleaning remaining clients.");
		for (std::map<int, ClientData*>::iterator it = _clients_map.begin(); it != _clients_map.end(); it++){
			it->second->close_fd();
			delete it->second;
			_clients_map.erase(it);
		}
	}
	_log->log(LOG_DEBUG, SM_NAME,
	          "Cleaning sockets servers.");
	for (size_t i = 0; i < _servers.size(); i++) {
		delete _servers[i];
	}
	_log->log(LOG_DEBUG, SM_NAME,
	          "Erasing from _polls_fds vector.");
	for (size_t i = 0; i < _poll_fds.size(); i++) {
		_poll_fds.erase(_poll_fds.begin() + (int)i);
	}
	_log->log(LOG_DEBUG, SM_NAME,
	          "Server Manager Resources Clean Up.");
}

void ServerManager::add_server(int port, ServerConfig& config) {
	try {
		SocketHandler* server = new SocketHandler(port, config, _log);
		_servers.push_back(server);
		_log->log(LOG_DEBUG, SM_NAME,
		          "SocketHandler instance created and added to _servers.");

		if (!add_server_to_poll(server->get_socket_fd())) {
			_log->log(LOG_ERROR, SM_NAME, "Failed to add server to poll list.");
			_servers.pop_back();
			delete server;
		}
	} catch (const SocketHandler::SocketCreationError &e) {
		_log->log(LOG_ERROR, SM_NAME, e.what());
		throw SocketHandler::SocketCreationError();
	} catch (const std::exception& e) {
		_log->log(LOG_ERROR, SM_NAME, "Error creating or adding SocketHandler: " + std::string(e.what()));
		throw ServerManager::ServerBuildError();
	}
}

bool ServerManager::add_server_to_poll(int server_fd) {
	if (server_fd < 0) {
    _log->log(LOG_ERROR, SM_NAME, "Invalid server file descriptor.");
    return (false);
    }
	for (size_t i = 0; i < _poll_fds.size(); ++i) {
		if (_poll_fds[i].fd == server_fd) {
		  _log->log(LOG_WARNING, SM_NAME, "Server fd already in _poll_fds.");
		  return (false);
		}
	}
	struct pollfd pfd;
	pfd.fd = server_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;

	_poll_fds.push_back(pfd);
	_log->log(LOG_DEBUG, SM_NAME, "Server fd added to _poll_fds.");
	return (true);
}

void ServerManager::turn_off_server() {
	_log->log(LOG_INFO, SM_NAME,
	          "Server it will be gracefully stop.");
	_active = false;
}

void ServerManager::run() {
	_log->log(LOG_DEBUG, SM_NAME, "Event loop started.");

	while (_active) {
		int poll_count = poll(&_poll_fds[0], _poll_fds.size(), -1);
		if (poll_count < 0 && _active) {
			_log->fatal_log(SM_NAME, "error in poll process.");
		}
		for (size_t i = 0; i < _poll_fds.size(); ++i) {
			if (_poll_fds[i].revents & POLLIN) {
				bool is_server = false;
				SocketHandler* server = NULL;
				// Check if the descriptor corresponds to a server socket
				for (size_t s = 0; s < _servers.size(); ++s) {
					if (_poll_fds[i].fd == _servers[s]->get_socket_fd()) {
						_log->log(LOG_DEBUG, SM_NAME, "fd belongs to a server.");
						is_server = true;
						server = _servers[s];
						break;
					}
				}
				if (is_server) {
					new_client(server);
				} else {
					// Handle client request
					process_request(i);
					--i;
				}
			}
		}
	}
}

bool    ServerManager::process_request(size_t poll_index) {
	int poll_fd = _poll_fds[poll_index].fd;

	std::map<int, ClientData*>::iterator it = _clients_map.find(poll_fd);
	if (it != _clients_map.end()) {
		HttpRequestHandler request_handler(_log, it->second);
		remove_client_from_poll(it, poll_index);
		return (true);
	} else {
		_log->log(LOG_WARNING, SM_NAME,
		          "No client was found at _client storage.");
		return (false);
	}
}

void    ServerManager::new_client(SocketHandler *server) {
	int client_fd = server->accept_connection();
	if (client_fd < 0) {
		_log->log(LOG_ERROR, SM_NAME, "Error getting client FD.");
		return;
	}
	ClientData* new_client = new ClientData(server, _log, client_fd);
	_clients_map.insert(std::make_pair(new_client->get_fd().fd, new_client));
	_poll_fds.push_back(new_client->get_fd());
	_log->log(LOG_DEBUG, SM_NAME,
	          "New Client accepted on port: " + server->get_port());
}

void    ServerManager::remove_client_from_poll(t_client_it client_data, size_t poll_index) {

	if (client_data != _clients_map.end()) {
		ClientData* current_client = client_data->second;
		_clients_map.erase(client_data);
		_poll_fds.erase(_poll_fds.begin() + (int)poll_index);
		current_client->close_fd();
		delete current_client;
		// Remove the client descriptor from _poll_fds
		_log->log(LOG_DEBUG, SM_NAME,
		          "fd remove from _polls_fds vector");
	} else {
		_log->log(LOG_ERROR, SM_NAME,
		          "Client was not found at clients storage.");
	}
}

const char* ServerManager::ServerBuildError::what() const throw() {
	return ("Error Building Server.");
}
