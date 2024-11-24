/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/24 02:33:07 by mporras-         ###   ########.fr       */
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
							 const Logger* logger):
							_log(logger) {
	if (_log == NULL) {
		throw Logger::NoLoggerPointer();
	}
	if (configs.empty()) {
		throw WebServerException("No configs available to create servers.");
	}
	_poll_fds.reserve(2000);
	_log->log_debug( SM_NAME,
			  "Server Manager Instance init.");
	std::ostringstream detail;
	try {
		build_servers(configs);
	} catch (const WebServerException& e) {
		detail << "Error Creating Servers: " << e.what();
		_log->log_error( SM_NAME,
				  detail.str());
		throw WebServerException(detail.str());
	} catch (const std::exception& e) {
		detail << "Error Creating Servers: " << e.what();
		_log->log_error( SM_NAME,
		          detail.str());
		throw WebServerException(detail.str());
	}
	_healthy = true;
	_active = true;
	_log->log_debug( SM_NAME,
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
	turn_off_server();
	_log->log_debug( SM_NAME,
	          "Server Manager Resources Clean Up.");
	_log->status(SM_NAME, "Server Resources Clean up.");
}

/**
 @section Functions to Build Server.
 */

/**
 * @brief Configures and builds servers based on the provided configurations.
 *
 * This method iterates through the given server configurations and performs the following actions:
 * 1. Checks if the port in the current configuration is already in use by an active server.
 * 2. If the port is not active:
 *    - Calls `add_server` to create a new server on the specified port.
 *    - Throws a `WebServerException` if server creation fails.
 * 3. If the port is already active:
 *    - Retrieves the server associated with the port and adds the host configuration to it.
 *
 * @param configs A vector of server configurations (`ServerConfig`) containing details such
 *                as ports, server names, and other necessary settings.
 *
 * @throws WebServerException If a new server cannot be created on any of the specified ports.
 *
 * @details
 * - Active servers are managed through the `_servers_map`, while active ports are tracked
 *   in `_active_ports`.
 *
 */
void ServerManager::build_servers(std::vector<ServerConfig> &configs) {
	_log->status(SM_NAME,
				 "Creating Server and hosts.");
	for (std::vector<ServerConfig>::iterator config_it = configs.begin(); config_it != configs.end(); config_it++) {
		std::map<int, int>::iterator listen_on = _active_ports.find(config_it->port);
		if (listen_on == _active_ports.end()) {
			if (!add_server(config_it->port, *config_it)) {
				throw WebServerException("Error creating new server.");
			}
		} else {
			SocketHandler* server = _servers_map[listen_on->second];
			server->add_host(*config_it);
		}
	}
}

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
 * @return true if the process end without errors, false otherwise
 */
bool ServerManager::add_server(int port, ServerConfig& config) {
	SocketHandler* server = new SocketHandler(port, config, _log);
	if (server == NULL) {
		return (false);
	}
	_log->log_debug( SM_NAME,
	          "SocketHandler instance created and added to _servers.");
	if (!add_server_to_poll(server->get_socket_fd())) {
		delete (server);
		return (false);
	}
	_servers_map[server->get_socket_fd()] = server;
	_active_ports[config.port] = server->get_socket_fd();
	return (true);
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
        _log->log_error( SM_NAME,
			  "Invalid server file descriptor.");
        return (false);
    }
	for (size_t i = 0; i < _poll_fds.size(); ++i) {
		if (_poll_fds[i].fd == server_fd) {
		  _log->log_warning( SM_NAME,
					"Server fd already in _poll_fds.");
		  return (false);
		}
	}
	struct pollfd pfd;
	pfd.fd = server_fd;
	pfd.events = POLLIN | POLLOUT;
	pfd.revents = 0;

	_poll_fds.push_back(pfd);
	_log->log_debug( SM_NAME, "Server fd added to _poll_fds.");
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
	_log->log_debug( SM_NAME,
			  "Cleaning up invalid file descriptors.");
	for (t_client_it it = _clients.begin(); it != _clients.end();) {
		if (fcntl(it->first, F_GETFD) == -1) {
			if (errno == EBADF) {
				_log->log_warning( SM_NAME,
				                   "Removing invalid file descriptor and its client.");
				t_client_it next = it++;
				remove_client_from_poll(it);
				it = next;
			} else {
				std::ostringstream detail;
				detail << "Unexpected error when checking fd." << strerror(errno);
				_log->log_error( SM_NAME,
				                 detail.str());
			}
		}
		it++;
	}
	_log->log_debug( SM_NAME,
			  "Cleanup completed.");
}

/**
 * @brief Removes clients whose connections have timed out.
 *
 * This method iterates over the `_timeout_index` map to identify clients
 * whose timestamps are earlier than or equal to the current time.
 * For each expired client, it removes the client’s file descriptor
 * from both `_clients` and the polling list.
 *
 * The method retrieves the current time in microseconds for precise
 * timeout calculations, and continues to remove timed-out clients until
 * it encounters a timestamp that is later than the current time or
 * exhausts the `_timeout_index`.
 *
 * @note This function relies on `gettimeofday` to provide time
 *       in seconds and microseconds.
 */
void ServerManager::timeout_clients() {
	if (_clients.empty()) {
		return;
	}
	struct timeval tv;
	gettimeofday(&tv, NULL);
	time_t current_time = tv.tv_sec * 1000000 + tv.tv_usec;
	t_timestamp_fd index_to;
	while (true) {
		index_to = _timeout_index.begin();
		if (index_to == _timeout_index.end() || index_to->first > current_time) {
			break;
		}
		int client_fd = index_to->second;
		t_client_it it_clients = _clients.find(client_fd);
		remove_client_from_poll(it_clients);
	}
}

/**
 * @brief Starts and manages the main event loop for the server.
 *
 * This method runs the server's event loop, continuously monitoring file descriptors for
 * events using `poll()`. It handles incoming connections, client requests, and outgoing
 * responses. The loop also manages timeouts for inactive clients and ensures the server
 * remains operational unless a critical error occurs.
 *
 * @throws WebServerException If a fatal error occurs that prevents the server from continuing.
 *
 * @details
 * The main functionalities of the event loop are:
 * - **Timeout Management:** Inactive clients are detected and removed using `timeout_clients`.
 * - **Polling for Events:** The `poll()` function is used to monitor file descriptors
 *   for readiness to read (`POLLIN`) or write (`POLLOUT`).
 * - **Handling Events:**
 *   - For `POLLIN`: Accepts new connections or processes incoming client requests.
 *   - For `POLLOUT`: Sends responses to clients.
 * - **Error Handling:** Handles errors from `poll()` such as `EINTR` (interrupted by a signal)
 *   or `EBADF` (bad file descriptor), logging warnings and cleaning up resources as needed.
 * - **Graceful Shutdown:** The loop exits when `_active` is set to `false`, ensuring that
 *   resources are properly cleaned up.
 *
 * @note
 * - The method ensures robustness by catching and logging exceptions, and by cleaning up
 *   invalid file descriptors if detected.
 * - A short sleep (`usleep`) is included to avoid excessive CPU usage during idle periods.
 *
 */
void ServerManager::run() {
	_log->log_debug( SM_NAME, "Event loop started.");
	_healthy = true;
	try {
		while (_active) {
			timeout_clients();
			usleep(700);
			int poll_count = poll(&_poll_fds[0], _poll_fds.size(), 100);
			if (poll_count == 0) {
				continue ;
			}
			if (poll_count < 0 && _active) {
				if (errno == EINTR) {
					_log->log_warning( SM_NAME,
					          "Poll interrupted by a signal, retrying.");
					continue ;
				} else if (errno == EBADF) {
					_log->log_warning( SM_NAME,
					          "Poll found a bad file descriptor.");
					cleanup_invalid_fds();
					continue ;
				} else {
					_log->log_error( RSP_NAME,
					          "Fatal error. Errno : " + int_to_string(errno));
					_healthy = false;
					break;
				}
			}
			for (size_t i = 0; i < _poll_fds.size(); ++i) {
				if (poll_count == 0) {
					break;
				}
				if (_poll_fds[i].revents & POLLOUT) {
					if (i > _servers_map.size() - 1) {
						process_response(i);
						poll_count--;
						continue;
					}
				} else if (_poll_fds[i].revents & POLLIN) {
					std::map<int, SocketHandler*>::iterator server_it = _servers_map.find(_poll_fds[i].fd);
					if (server_it != _servers_map.end()) {
						while (new_client(server_it->second)) {
							poll_count--;
						};
					} else {
						process_request(i);
						poll_count--;
						continue;
					}
				}
			}
		}
	} catch (std::exception& e) {
		std::ostringstream detail;
		if (!_active) {
			detail << "Killing by user signal: " << e.what();
			_log->log_error( SM_NAME,
					  detail.str());
		} else {
			detail << "Fatal Error: " << e.what();
			_log->log_error( SM_NAME,
					  detail.str());
		}
		throw WebServerException(detail.str());
	}
	if (!_healthy) {
		_log->log_error( SM_NAME,
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
bool    ServerManager::new_client(SocketHandler *server) {
	int client_fd = server->accept_connection();
	if (client_fd < 0) {
		return (false);
	}
	ClientData* new_client = new ClientData(server, _log, client_fd);
	int fd = new_client->get_fd().fd;
	_clients[fd] = new_client;
	_poll_fds.push_back(new_client->get_fd());
	_poll_index[fd] = _poll_fds.size() - 1;
	time_t timestamp = timeout_timestamp();
	_timeout_index[timestamp] = fd;
	_index_timeout[fd] = timestamp;
	_log->log_debug( SM_NAME,
	          "New Client accepted on port: " + server->get_port());
	return (true);
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
			HttpRequestHandler request_handler(_log, it->second);
			request_handler.request_workflow();
			if (!it->second->is_alive()) {
				remove_client_from_poll(it);
				--poll_index;
			} else {
				_poll_fds[poll_index].events = POLLOUT;
				_poll_fds[poll_index].revents = 0;
				int fd = it->second->get_fd().fd;
				time_t new_cycle = timeout_timestamp();
				t_fd_timestamp index_timeout = _index_timeout.find(fd);
				t_timestamp_fd timeout_index = _timeout_index.find(index_timeout->second);
				_timeout_index.erase(timeout_index);
				_timeout_index[new_cycle] = fd;
				_index_timeout[fd] = new_cycle;
			}
			return (true);
		} else {
			_log->log_warning( SM_NAME,
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
 */
void    ServerManager::remove_client_from_poll(t_client_it client_data) {
	if (client_data != _clients.end()) {
		std::map<int, size_t>::iterator index_it = _poll_index.find(client_data->second->get_fd().fd);
		size_t index = index_it->second;
		std::map<int, time_t>::iterator index_to_it = _index_timeout.find(client_data->second->get_fd().fd);
		std::map<time_t, int>::iterator to_index_it = _timeout_index.find(index_to_it->second);

		if (index < _poll_fds.size() - 1) {
			std::swap(_poll_fds[index], _poll_fds.back());
			int swapped_fd = _poll_fds[index].fd;
			_poll_index[swapped_fd] = index;
		}
		_poll_fds.pop_back();
		delete client_data->second;
		_clients.erase(client_data);
		_poll_index.erase(index_it);
		_timeout_index.erase(to_index_it);
		_index_timeout.erase(index_to_it);
	}
}

bool ServerManager::process_response(size_t& poll_index) {
	int poll_fd = _poll_fds[poll_index].fd;
	std::map<int, ClientData*>::iterator it = _clients.find(poll_fd);
	if (it == _clients.end()) {
		return (false);
	}
	ClientData* client = it->second;
	try {
		s_request& request = client->client_request();
//		SocketHandler* server = client->get_server();
		if (!request.sanity){
			HttpResponseHandler response(request.location, _log, client, request, poll_fd);
			response.send_error_response();
		}
		else if (request.factory == 0) {
			HttpResponseHandler response(request.location, _log, client, request, poll_fd);
			response.handle_request();
//			WebServerCache<CacheRequest>& request_cache = server->get_request_cache();
//			if (request.is_cached) {
//				if (!request.sanity) {
//					request_cache.remove(request.path);
//				}
//			}
//			if (request.sanity && HAS_GET(request.method)) {
//				request_cache.put(request.path,
//				                   CacheRequest(request.path, request.host_config,
//				                                request.location, request.normalized_path));
//			}
		}
		else if (request.is_redir) {
			HttpResponseHandler response(request.location, _log, client, request, poll_fd);
			response.redirection();
		}
		else if (request.autoindex) {
			HttpAutoIndex response(request.location, _log, client, request, poll_fd);
			response.handle_request();
		}
		else if (request.cgi) {
			HttpCGIHandler response(request.location, _log, client, request, poll_fd);
			response.handle_request();
			client->deactivate();
		} else if (!request.range.empty()){
			HttpRangeHandler response(request.location, _log, client, request, poll_fd);
			response.handle_request();
		} else if (!request.boundary.empty()){
			HttpMultipartHandler response(request.location, _log, client, request, poll_fd);
			response.handle_request();
		}
		if (client->is_active() && client->is_alive()) {
			_poll_fds[poll_index].events = POLLIN;
			time_t new_cycle = timeout_timestamp();
			t_fd_timestamp index_timeout = _index_timeout.find(poll_fd);
			t_timestamp_fd timeout_index = _timeout_index.find(index_timeout->second);
			_timeout_index.erase(timeout_index);
			_timeout_index[new_cycle] = poll_fd;
			_index_timeout[poll_fd] = new_cycle;
			return (true);
		}
	} catch (WebServerException& e) {
		std::ostringstream detail;
		detail << "Error Handling response: " << e.what();
		_log->log_error( RH_NAME,
		                 detail.str());
		client->deactivate();
	} catch (Logger::NoLoggerPointer& e) {
		std::ostringstream detail;
		detail << "Logger Pointer Error at Response Handler. "
		       << "Server Sanity could be compromise.";
		_log->log_error( RH_NAME,
		                 detail.str());
		client->deactivate();
	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Unknown error handling response: " << e.what();
		_log->log_error( RH_NAME,
		                 detail.str());
		client->deactivate();
	}
	remove_client_from_poll(it);
	--poll_index;
	return (false);
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
	_log->log_error( SM_NAME, detail);
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
	_log->log_info( SM_NAME, "Server shutdown initiated.");
	_active = false;
	_healthy = false;
	clear_clients();
	clear_servers();
	clear_poll();
	_log->log_info( SM_NAME, "Server shutdown completed.");
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
			_log->log_debug( SM_NAME,
			          "ClientData Cleared.");
		} catch (std::exception &e) {
			std::ostringstream details;
			details << "Error closing Clearing Client Data: " << e.what();
			_log->log_error( SM_NAME,
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
		for (std::map<int, SocketHandler*>::iterator it = _servers_map.begin();it != _servers_map.end(); it++) {
			delete it->second;
		}
		_servers_map.clear();
		_active_ports.clear();
		_log->log_debug( SM_NAME,
						 "Servers cleared successfully.");
	} catch (std::exception& e) {
		_log->log_error( SM_NAME,
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
			it++;
		}
		_poll_fds.clear();
	} catch (std::exception& e) {
		_log->log_error( SM_NAME,
		          "Error clearing poll.");
	}
}

/**
 * @brief Calculates a timeout timestamp with microsecond precision.
 *
 * This method retrieves the current time in microseconds and adds
 * a predefined client lifecycle duration to set an expiration time.
 *
 * @return The timeout timestamp in microseconds from the epoch,
 *         adjusted by CLIENT_LIFECYCLE.
 *
 * @note This function uses `gettimeofday`, which provides time
 *       in seconds and microseconds, making it suitable for precise
 *       timeout calculations.
 */
time_t ServerManager::timeout_timestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000 + tv.tv_usec + CLIENT_LIFECYCLE);
}
