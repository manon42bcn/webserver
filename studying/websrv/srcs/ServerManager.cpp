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
 * @brief Constructs a new ServerManager instance.
 *
 * This constructor initializes the `ServerManager` by setting up server instances
 * for each configuration provided in the `configs` vector. It reserves space for
 * polling file descriptors (_poll_fds) and associates a logger for logging activities
 * related to server management.
 *
 * @param configs A vector of `ServerConfig` structures that define the configurations
 * for each server (such as port, document root, error pages, etc.).
 * @param logger A pointer to a `Logger` instance used for logging server events.
 *
 * @details
 * - The constructor reserves space for polling file descriptors to optimize memory
 *   allocations, especially if a large number of servers is expected.
 * - Each server is added by calling `add_server()`, which creates and binds the server
 *   sockets.
 * - If any errors occur during the server setup, they are logged.
 *
 * @exception None explicitly thrown, but the logger may handle critical errors that could
 * result in the program terminating.
 */
ServerManager::ServerManager(const std::vector<ServerConfig>& configs, const Logger* logger):
							_log(logger) {
	if (_log == NULL) {
		throw Logger::NoLoggerPointer();
	}
        _poll_fds.reserve(100);
	for (size_t i = 0; i < configs.size(); ++i) {
		add_server(configs[i].port, configs[i]);
	}
	_log->log(LOG_DEBUG, SM_NAME, "instance init and ready.");
}

/**
 * @brief Destructor for the ServerManager class.
 *
 * This destructor ensures that all resources are properly cleaned up
 * when the server shuts down, including:
 * - Closing all _poll_fds descriptors.
 * - Deleting dynamically allocated objects like SocketHandlers and Logger.
 *
 * This destructor should be explicitly called when the server is about to exit
 * to ensure proper cleanup, especially if `exit()` or a similar function is used,
 * as destructors will not be called automatically in those cases.
 *
 * @param None
 * @return None
 */

ServerManager::~ServerManager() {
	for (size_t i = 0; i < _poll_fds.size(); i++)
	{
		if (_poll_fds[i].fd >= 0) {
			close(_poll_fds[i].fd);
		}
	}
	for (size_t i = 0; i < _servers.size(); i++)
		delete _servers[i];
	_log->log(LOG_DEBUG, SM_NAME, "ServerManager resources cleaned up.");
}

/**
 * @brief Adds a new server to the `ServerManager`.
 *
 * This method creates a new `SocketHandler` instance for the specified port and configuration.
 * The server is added to the `_servers` vector and its file descriptor is added to the poll list
 * (_poll_fds) for event monitoring.
 *
 * @details
 * - If an error occurs while creating the `SocketHandler` or adding the server to the poll list,
 *   an error log is generated. In such cases, the server instance is deleted to avoid memory leaks.
 * - Future extensions could include checking for duplicate servers on the same port before adding
 *   a new one.
 *
 * @param port The port number on which the new server will listen.
 * @param config The configuration object (`ServerConfig`) for the server.
 *
 * @return None
 */
void ServerManager::add_server(int port, const ServerConfig& config) {
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
	} catch (const std::exception& e) {
		_log->log(LOG_ERROR, SM_NAME, "Error creating or adding SocketHandler: " + std::string(e.what()));
	}
}


/**
 * @brief Adds a server's file descriptor to the poll list.
 *
 * This method adds a server's file descriptor to the `_poll_fds` vector to monitor it for incoming
 * connections using the `poll()` system call.
 *
 * @details
 * - Before adding the file descriptor, the method checks if it is already present in the `_poll_fds`
 *   vector to avoid duplicates.
 * - It logs an error if the file descriptor is invalid (less than 0) or if the descriptor is already
 *   in the poll list.
 *
 * @param server_fd The file descriptor of the server socket to be monitored.
 * @return bool True if the file descriptor was successfully added, false otherwise.
 */
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

/**
 * @brief Runs the main event loop for the server, handling client connections and requests.
 *
 * This method starts an infinite loop that monitors server and client file descriptors
 * using the `poll()` system call. When a server file descriptor signals an incoming
 * connection, the method accepts the connection and adds the new client to the list of
 * monitored clients. When a client file descriptor signals activity, the method creates
 * an `HttpRequestHandler` instance to process the client's request.
 *
 * @details
 * - Server sockets are identified by comparing file descriptors with those stored in the
 *   `_servers` vector. If a match is found, the socket is assumed to be a server socket,
 *   and a new client connection is accepted.
 * - Client file descriptors are handled by looking them up in the `_clients` vector.
 * - After a client request is processed, the client connection is closed, and the client
 *   is removed from the `_poll_fds` and `_clients` vectors.
 *
 * Error handling:
 * - If `poll()` fails, a fatal log is generated and the server will be terminatet.
 * - If `accept_connection()` returns an invalid file descriptor, the connection is rejected.
 *
 * @todo check for fd leaks, may be is important to close gently the server on error.
 * @note This method currently handles only `POLLIN` events, which means it only reacts to
 * readable events. Future extensions could include handling other events, such as `POLLOUT`
 * for writable sockets.
 *
 * @exception None directly thrown by this method, but low-level system errors could cause
 * the server to log fatal messages and possibly terminate.
 *
 * @param None
 * @return None
 */
void ServerManager::run() {
	_log->log(LOG_DEBUG, SM_NAME, "Event loop started.");
	while (true) {
		int poll_count = poll(&_poll_fds[0], _poll_fds.size(), -1);
		if (poll_count < 0)
			_log->fatal_log(SM_NAME, "error in poll process.");
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
					// Accept a new connection, create client and
					// append fds to _poll_fds and _clients
					new_client(server);
				} else {
					// Handle client request
					process_request(_poll_fds[i].fd);

					// Close the client connection
					close(_poll_fds[i].fd);
					_log->log(LOG_DEBUG, SM_NAME,
					          "close connection: " + int_to_string((int)i));
					// Remove the client descriptor from _poll_fds
					_poll_fds.erase(_poll_fds.begin() + (int)i);
					_log->log(LOG_DEBUG, SM_NAME,
					          "fd remove from _polls_fds vector: " + int_to_string((int)i));
					--i;  // Adjust index to check the new descriptor in this position
				}
			}
		}
	}
}

/**
 * @brief Processes a request from a client identified by a poll file descriptor.
 *
 * This method checks if the given `poll_fd` corresponds to an existing client. If a match
 * is found, it creates an `HttpRequestHandler` to handle the client's request and then
 * removes the client from the polling list. If no match is found, a log entry is made.
 *
 * @details
 * - The method iterates over the `_clients` vector to find the client corresponding to `poll_fd`.
 *   If a match is found, the client is processed and removed from polling.
 * - If the `poll_fd` no matching client is found, appropriate log messages are generated.
 *
 * @param poll_fd The file descriptor returned by the `poll()` system call.
 * @return bool True if the client request was successfully processed, false otherwise.
 */
bool    ServerManager::process_request(int poll_fd) {
	// TODO: for performance it will be a good idea use _client as map
	// Ive found some issues with fd..
	std::map<int, ClientData>::iterator it = _clients_map.find(poll_fd);
	if (it != _clients_map.end()) {
		HttpRequestHandler request_handler(_log, it->second);
		remove_client_from_poll(poll_fd);
		_log->log(LOG_DEBUG, SM_NAME,
		          "End of request process and client removed from poll.");
		return (true);
		_log->log(LOG_ERROR, SM_NAME, "From map " + int_to_string(it->second.get_fd().fd));
	} else {
		for (size_t c = 0; c < _clients.size(); ++c) {
			if (poll_fd == _clients[c].get_fd().fd) {
				HttpRequestHandler request_handler(_log, _clients[c]);
				remove_client_from_poll(poll_fd);
				_log->log(LOG_DEBUG, SM_NAME,
				          "End of request process and client removed from poll.");
				return (true);
			}
		}
	}
	_log->log(LOG_INFO, SM_NAME,
	          "No client was found at _client storage.");
	return (false);
}

/**
 * @brief Accepts a new client connection and adds it to monitoring lists.
 *
 * This method accepts a new client connection from the provided `SocketHandler` (server),
 * creates a `ClientData` object to track the client, and adds it to both the `_clients`
 * vector and the `_poll_fds` vector for event monitoring.
 *
 * @details
 * - The method calls `accept_connection()` on the server to get the client file descriptor (`client_fd`).
 *   If the `client_fd` is invalid (negative), an error is logged, and the method returns.
 * - The newly created `ClientData` object is added to the `_clients` and `_poll_fds` vectors
 *   to monitor the client and handle future requests.
 * - Logs are generated to track the acceptance of the new client, including the port and the `client_fd`.
 *
 * @param server A pointer to the `SocketHandler` instance that accepted the client.
 * @return None
 */
void    ServerManager::new_client(SocketHandler *server) {
	int client_fd = server->accept_connection();
	if (client_fd < 0) {
		_log->log(LOG_ERROR, SM_NAME, "Error getting client FD.");
		return;
	}
	ClientData new_client(server, _log, client_fd);
	_clients_map.insert(std::make_pair(new_client.get_fd().fd, new_client));
	_clients.push_back(new_client);
	_poll_fds.push_back(new_client.get_fd());
	_log->log(LOG_DEBUG, SM_NAME,
	          "New Client accepted on port " + int_to_string(server->get_socket_fd()));
}

/**
 * @brief Removes a client from the clients vector based on its file descriptor.
 *
 * This method searches for a client in the `_clients` vector whose file descriptor (`fd`)
 * matches the provided `fd`. If the client is found, it is removed from the vector.
 *
 * @details
 * - The method iterates through the `_clients` vector to find the matching client.
 * - Once found, the client is removed from the vector, and a log message is generated
 *   indicating the successful removal.
 * - If the client is not found, an error log is generated to indicate that the client
 *   could not be located in the vector.
 *
 * @param fd The file descriptor of the client to remove.
 * @return None
 */
void    ServerManager::remove_client_from_poll(int fd) {
	std::map<int, ClientData>::iterator it = _clients_map.find(fd);
	if (it != _clients_map.end()) {
		_log->log(LOG_DEBUG, SM_NAME, it->second.saludo());
		_clients_map.erase(it);
		_log->log(LOG_DEBUG, SM_NAME,
		          "End of request process and client removed from poll.");
		return ;
	} else {
		return ;
	}

	for (size_t c = 0; c < _clients.size(); ++c) {
		if (_clients[c].get_fd().fd == fd) {
			_clients.erase(_clients.begin() + (int)c);
			_log->log(LOG_DEBUG, SM_NAME, "client remove from _client vector.");
			break;
		}
	}
	_log->log(LOG_ERROR, SM_NAME,
	          "Client was not found at clients storage.");
}

