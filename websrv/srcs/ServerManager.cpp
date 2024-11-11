/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/11 14:20:44 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerManager.hpp"

/**
 * @brief Constructs a ServerManager instance, initializing server configurations and socket handlers.
 *
 * This constructor initializes the ServerManager with the provided server configurations, logger, and cache.
 * It creates server instances based on the configurations, setting up socket connections and handling
 * potential exceptions related to socket creation, linking, listening, and non-blocking mode.
 *
 * @param configs Vector of server configurations, each containing port and other server settings.
 * @param logger Pointer to the Logger instance for logging activities within the ServerManager.
 * @param cache Pointer to the WebServerCache for caching resources.
 *
 * @throws Logger::NoLoggerPointer If the logger pointer is null.
 * @throws WebServerException If no cache pointer is provided, if configs are empty, or if a SocketHandler exception is caught.
 *
 * This constructor sets up sockets for each configuration in `configs` using `add_server`.
 * If any socket operation fails, it logs the error details and throws a `WebServerException`.
 * Once initialized successfully, the instance is marked as healthy and active.
 */
ServerManager::ServerManager(std::vector<ServerConfig>& configs,
							 const Logger* logger,
							 WebServerCache* cache):
							_log(logger),
							_cache(cache) {
	if (_log == NULL) {
		throw Logger::NoLoggerPointer();
	}
	if (!cache) {
		throw WebServerException("No valid pointer to webserver cache.");
	}
	if (configs.empty()) {
		throw WebServerException("No configs available to create servers.");
	}
	_poll_fds.reserve(3000);
	_log->log(LOG_DEBUG, SM_NAME,
			  "Server Manager Instance init.");
	std::ostringstream detail;
	try {
		for (size_t i = 0; i < configs.size(); ++i) {
			add_server(configs[i].port, configs[i]);
		}
	} catch (const WebServerException& e) {
		detail << "Error Creating Servers: " << e.what();
		_log->log(LOG_ERROR, SM_NAME,
				  detail.str());
		throw WebServerException(detail.str());
	} catch (const std::exception& e) {
		detail << "Error Creating Servers: " << e.what();
		_log->log(LOG_ERROR, SM_NAME,
		          detail.str());
		throw WebServerException(detail.str());
	}
	_healthy = true;
	_active = true;
	_log->log(LOG_DEBUG, SM_NAME,
			  "instance init and ready.");
	_log->status(SM_NAME, "ServerManager Instance Ready.");
}

/**
 * @brief Destructor for the ServerManager class, responsible for resource cleanup.
 *
 * This destructor ensures a proper cleanup of server resources managed by the ServerManager instance.
 * It deactivates and clears client connections, server instances, and poll file descriptors,
 * releasing any associated resources.
 *
 * The destructor also logs the cleanup process, marking the completion of resource cleanup for
 * easier monitoring and debugging.
 */
ServerManager::~ServerManager() {
	clear_clients();
	clear_servers();
	clear_poll();
	_log->log(LOG_DEBUG, SM_NAME,
	          "Server Manager Resources Clean Up.");
	_log->status(SM_NAME, "Server Resources Clean up.");
}

/**
 @section Functions to Build Server.
 */

/**
* @brief Adds a new server instance to the manager and its poll list.
*
* This method creates a new `SocketHandler` instance for the specified port and configuration
* and adds it to the `_servers` vector. The method then attempts to add the server's
 * socket file descriptor to the poll list for monitoring. If this step fails, the server
 * is removed from `_servers` and deleted to prevent memory leaks.
 *
 * @param port Port number for the server.
 * @param config Configuration settings for the server.
 *
 * Logs the creation of each `SocketHandler` and any failures in adding it to the poll list.
 */
void ServerManager::add_server(int port, ServerConfig& config) {

	SocketHandler* server = new SocketHandler(port, config, _log);
	_servers.push_back(server);
	_log->log(LOG_DEBUG, SM_NAME,
	          "SocketHandler instance created and added to _servers.");

	if (!add_server_to_poll(server->get_socket_fd())) {
		_log->log(LOG_ERROR, SM_NAME,
				  "Failed to add server to poll list.");
		_servers.pop_back();
		delete (server);
	}
}

/**
 * @brief Adds a server file descriptor to the poll list for monitoring.
 *
 * This method verifies that the provided server file descriptor is valid and not
 * already present in the `_poll_fds` list. If valid, it creates a new `pollfd`
 * structure for the file descriptor, configures it to listen for incoming events,
 * and adds it to the `_poll_fds` list.
 *
 * @param server_fd The server file descriptor to be added to the poll list.
 * @return `true` if the server file descriptor was successfully added; `false` if it
 *         was invalid or already in the poll list.
 *
 * Logs an error if the file descriptor is invalid, a warning if it is already in `_poll_fds`,
 * and a debug message on successful addition.
 */
bool ServerManager::add_server_to_poll(int server_fd) {
	if (server_fd < 0) {
        _log->log(LOG_ERROR, SM_NAME,
			  "Invalid server file descriptor.");
        return (false);
    }
	for (size_t i = 0; i < _poll_fds.size(); ++i) {
		if (_poll_fds[i].fd == server_fd) {
		  _log->log(LOG_WARNING, SM_NAME,
					"Server fd already in _poll_fds.");
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
 @section Core Functions
 */

/**
 * @brief Cleans up invalid file descriptors from the poll list and associated clients.
 *
 * This method iterates through the `_poll_fds` vector, checking each file descriptor’s validity
 * using `fcntl`. If a file descriptor is invalid (with `errno` set to `EBADF`), it logs a warning
 * and removes the associated client and poll entry. For any other `fcntl` errors, an error message
 * is logged with the corresponding error details.
 *
 * This process helps maintain a clean and accurate poll list, removing any defunct or closed connections.
 */
void ServerManager::cleanup_invalid_fds() {
	_log->log(LOG_DEBUG, SM_NAME,
			  "Cleaning up invalid file descriptors.");
	for (size_t i = 0; i < _poll_fds.size(); i++) {
		if (fcntl(_poll_fds[i].fd, F_GETFD) == -1) {
			if (errno == EBADF) {
				_log->log(LOG_WARNING, SM_NAME,
						  "Removing invalid file descriptor and its client.");
				t_client_it it = _clients.find(_poll_fds[i].fd);
				remove_client_from_poll(it, i);
			} else {
				std::ostringstream detail;
				detail << "Unexpected error when checking fd." << strerror(errno);
				_log->log(LOG_ERROR, SM_NAME,
						  detail.str());
			}
		}
	}
	_log->log(LOG_DEBUG, SM_NAME,
			  "Cleanup completed.");
}

/**
 * @brief Removes clients that have exceeded the timeout duration.
 *
 * This method iterates through the `_poll_fds` vector, checking each associated client’s
 * connection status. If a client’s connection has timed out, it removes the client
 * and its poll entry from `_poll_fds` to maintain a clean list of active connections.
 *
 * The `chronos_connection` method is used to determine if a client has timed out, and
 * if so, `remove_client_from_poll` is called to handle the cleanup.
 */
void ServerManager::timeout_clients() {
	if (_clients.empty()) {
		return;
	}
	for (size_t i = 0; i < _poll_fds.size(); i++) {
		t_client_it it = _clients.find(_poll_fds[i].fd);
		if (it != _clients.end() && !it->second->chronos_connection()) {
			remove_client_from_poll(it, i);
		}
	}
}

/**
 * @brief Main event loop for managing client connections and processing requests.
 *
 * This method runs an event loop that handles timeouts, polling, and request processing
 * for all active connections. The loop will continue executing as long as `_active` remains true.
 *
 * - The method begins by checking and removing any timed-out clients.
 * - It then polls the `_poll_fds` list for incoming events on file descriptors.
 * - If a server socket receives an event, a new client is accepted.
 * - If a client connection receives an event, the request is processed.
 *
 * In case of errors:
 * - If `poll` is interrupted by a signal, it logs a warning and retries.
 * - For `EBADF` errors, it attempts to clean up invalid file descriptors and continue.
 * - For unrecoverable errors, it logs the error and sets `_healthy` to false.
 *
 * @throws WebServerException If an unrecoverable error occurs, the exception is thrown with a detailed message.
 */
void ServerManager::run() {
	_log->log(LOG_DEBUG, SM_NAME, "Event loop started.");
	_healthy = true;
	try {
		while (_active) {
			timeout_clients();
			int poll_count = poll(&_poll_fds[0], _poll_fds.size(), -1);
			if (poll_count < 0 && _active) {
				if (errno == EINTR) {
					_log->log(LOG_WARNING, SM_NAME,
					          "Poll interrupted by a signal, retrying.");
					continue ;
				} else if (errno == EBADF) {
					_log->log(LOG_WARNING, SM_NAME,
					          "Poll found a bad file descriptor.");
					cleanup_invalid_fds();
					continue ;
				} else {
					_log->log(LOG_ERROR, RSP_NAME,
					          "Fatal error. Errno : " + int_to_string(errno));
					_healthy = false;
					break;
				}
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
						process_request(i);
					}
				}
			}
		}
	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Fatal Error: " << e.what();
		_log->log(LOG_ERROR, SM_NAME,
				  detail.str());
		throw WebServerException(detail.str());
	}
	if (!_healthy) {
		_log->log(LOG_ERROR, SM_NAME,
				  "Unrecoverable Error. Check Log for details.");
		throw WebServerException("Unrecoverable Error. Check Log for details.");
	}
}

/**
 * @brief Accepts a new client connection from the specified server and initializes the client data.
 *
 * This method uses the `SocketHandler` instance to accept a new client connection. If the connection
 * is successfully established, a `ClientData` instance is created to manage the client’s data and
 * state. The new client’s file descriptor is then added to both `_clients` and `_poll_fds` for
 * tracking and polling purposes.
 *
 * - Logs an error if the client file descriptor is invalid.
 * - Upon success, logs the acceptance of the new client, including the server port.
 *
 * @param server Pointer to the `SocketHandler` instance representing the server that accepted the new client.
 */
void    ServerManager::new_client(SocketHandler *server) {
	int client_fd = server->accept_connection();
	if (client_fd < 0) {
		_log->log(LOG_ERROR, SM_NAME, "Error getting client FD.");
		return;
	}
	ClientData* new_client = new ClientData(server, _log, client_fd);
	_clients.insert(std::make_pair(new_client->get_fd().fd, new_client));
	_poll_fds.push_back(new_client->get_fd());
	_log->log(LOG_DEBUG, SM_NAME,
	          "New Client accepted on port: " + server->get_port());
}

/**
 * @brief Processes a client request based on the file descriptor at the specified poll index.
 *
 * This method checks the `_clients` map for a client associated with the file descriptor at
 * `poll_index` in `_poll_fds`. If the client is found, it instantiates an `HttpRequestHandler` to
 * manage the request workflow.
 *
 * - If the client is no longer active after processing, the client is removed from `_poll_fds`.
 * - If the client remains active, its timestamp is reset via `chronos_reset`.
 * - Catches various exceptions and logs errors accordingly. If a critical error occurs, it logs
 *   the error message and initiates a safe server shutdown by setting `_healthy` to false.
 *
 * @param poll_index Index of the file descriptor in `_poll_fds` to process.
 * @return true if the request was successfully processed and the client remains active;
 *         false if the client was not found or removed from the poll list.
 */
bool    ServerManager::process_request(size_t& poll_index) {
	try {
		int poll_fd = _poll_fds[poll_index].fd;
		std::map<int, ClientData*>::iterator it = _clients.find(poll_fd);
		if (it != _clients.end()) {
			HttpRequestHandler request_handler(_log, it->second, _cache);
			request_handler.request_workflow();
			if (!it->second->is_active() || !it->second->is_alive()) {
				remove_client_from_poll(it, poll_index);
			} else {
				it->second->chronos_reset();
			}
			return (true);
		} else {
			_log->log(LOG_WARNING, SM_NAME,
			          "No client was found at _client storage.");
			return (false);
		}
	} catch (WebServerException& e) {
		std::ostringstream detail;
		detail << "Error Building Request. Server Health can be compromised." << e.what()
		       << "\n. Server is set to shut down safely.";
		return (turn_off_sanity(detail.str()));
	} catch (Logger::NoLoggerPointer& e) {
		std::ostringstream detail;
		detail << "No pointer Error. Server Health can be compromised: " << e.what()
			   << "\n. Server is set to shut down safely.";
		return (turn_off_sanity(detail.str()));
	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Unknown Exception. Server Health can be compromised." << e.what()
			   << "\n. Server is set to shut down safely.";
		return (turn_off_sanity(detail.str()));
	}
}

/**
 * @brief Removes a client from the `_clients` map and `_poll_fds` vector, closes the client’s file descriptor, and deallocates client resources.
 *
 * This method locates the client data by its iterator `client_data` in the `_clients` map and removes it from both `_clients` and `_poll_fds`.
 * - Closes the client’s file descriptor by calling `ClientData::close_fd`.
 * - Deletes the `ClientData` instance, freeing resources.
 * - Decrements `poll_index` to maintain the correct index in `_poll_fds` after removal.
 * - Logs successful removal or an error if the client was not found.
 *
 * @param client_data Iterator pointing to the client’s data in the `_clients` map.
 * @param poll_index Index in `_poll_fds` where the client’s poll descriptor is stored.
 */
void    ServerManager::remove_client_from_poll(t_client_it client_data, size_t& poll_index) {

	if (client_data != _clients.end()) {
		ClientData* current_client = client_data->second;
		_clients.erase(client_data);
		_poll_fds.erase(_poll_fds.begin() + (int)poll_index);
		current_client->close_fd();
		delete current_client;
		--poll_index;
		_log->log(LOG_DEBUG, SM_NAME,
		          "fd remove from _polls_fds vector");
	} else {
		_log->log(LOG_ERROR, SM_NAME,
		          "Client was not found at clients storage.");
	}
}

/**
 * @brief Deactivates the server and logs a critical error, indicating an unrecoverable issue.
 *
 * This method is used to handle critical errors that require the server to stop running. It logs the error detail
 * at the ERROR level, sets `_healthy` to `false` to mark the server's state as unhealthy, and sets `_active` to
 * `false` to stop the server's main loop.
 *
 * - Logs the error details and updates the server's status to reflect the critical failure.
 *
 * @param detail A string describing the reason for turning off server sanity.
 * @return Always returns `false` to signal failure.
 */
bool ServerManager::turn_off_sanity(const std::string &detail) {
	_log->log(LOG_ERROR, SM_NAME, detail);
	_healthy = false;
	_active = false;
	_log->status(SM_NAME, detail);
	return (false);
}

/**
 @section Functions to Clear resources.
 */

/**
 * @brief Turn off server.
 *
 * This method is be used by signal handler.
 */
void ServerManager::turn_off_server() {
	_log->log(LOG_INFO, SM_NAME, "Server shutdown initiated.");
	_active = false;
	_healthy = false;
	
	// Primero cerrar todos los clientes
	clear_clients();
	
	// Luego cerrar todos los servidores
	clear_servers();
	
	// Finalmente limpiar los poll_fds
	clear_poll();
	
	_log->log(LOG_INFO, SM_NAME, "Server shutdown completed.");
}

/**
 * @brief Clears and deallocates all client data managed by the ServerManager.
 *
 * This method iterates through the `_clients` container, deleting each `ClientData` instance
 * to free associated resources and prevent memory leaks. After each client is deleted, it is removed
 * from the `_clients` container. If an exception occurs during deletion, an error is logged with details.
 *
 * The method logs the successful clearing of client data upon completion.
 */
void ServerManager::clear_clients() {
	if (!_clients.empty()) {
		try {
			for (t_client_it it = _clients.begin(); it != _clients.end(); it++) {
				ClientData* data = it->second;
				_clients.erase(it);
				delete data;
			}
			_log->log(LOG_DEBUG, SM_NAME,
			          "ClientData Cleared.");
		} catch (std::exception &e) {
			std::ostringstream details;
			details << "Error closing Clearing Client Data: " << e.what();
			_log->log(LOG_ERROR, SM_NAME,
			          details.str());
		}
	}
}

/**
 * @brief Clears and deallocates all server instances managed by the ServerManager.
 *
 * This method iterates through the `_servers` container, deleting each server instance
 * to release associated resources and prevent memory leaks. Upon successful completion,
 * the method logs a message indicating that all servers were cleared.
 *
 * If an exception occurs during the deletion process, an error message is logged with
 * details to aid in debugging.
 */
void ServerManager::clear_servers() {
	try {
		for (std::vector<SocketHandler*>::iterator it = _servers.begin();
			 it != _servers.end(); ++it) {
			delete *it;
		}
		_servers.clear();
		_log->log(LOG_DEBUG, SM_NAME, "Servers cleared successfully.");
	} catch (std::exception& e) {
		_log->log(LOG_ERROR, SM_NAME,
				  "Error clearing servers: " + std::string(e.what()));
	}
}

/**
 * @brief Clears and closes all file descriptors in the `_poll_fds` vector.
 *
 * This method iterates through the `_poll_fds` vector, retrieving and checking each
 * file descriptor's flags to ensure validity. If the file descriptor is open and valid,
 * it closes the descriptor to release resources. Each file descriptor is then removed
 * from `_poll_fds`.
 *
 * If an exception occurs during the process, an error message is logged.
 */
void ServerManager::clear_poll() {
	try {
		std::vector<struct pollfd>::iterator it = _poll_fds.begin();
		while (it != _poll_fds.end()) {
			int flags;
			flags = fcntl(it->fd, F_GETFL);
			if (flags != -1 && errno != EBADF) {
				close(it->fd);
			}
			it = _poll_fds.erase(it);
		}
	} catch (std::exception& e) {
		_log->log(LOG_ERROR, SM_NAME,
		          "Error clearing poll.");
	}
}
