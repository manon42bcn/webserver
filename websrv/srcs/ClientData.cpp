/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientData.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 08:43:27 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/25 23:03:05 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "ClientData.hpp"

/**
 * @brief Constructs a `ClientData` instance for a client connection.
 *
 * Initializes the client data with the provided server socket handler, logger,
 * and client file descriptor. Sets initial connection state to active and
 * timestamps the connection for monitoring.
 *
 * @param server Pointer to the server's `SocketHandler`, responsible for managing the client connection.
 * @param log Pointer to the `Logger` instance for recording client activities.
 * @param fd File descriptor for the client's socket, set to monitor read events (`POLLIN`).
 */
ClientData::ClientData(SocketHandler* server,
					   const Logger* log, int fd):
					   _server(server),
					   _log(log),
					   _active(false),
					   _alive(true),
					   _client_fd() {

	_client_fd.fd = fd;
	_client_fd.events = POLLIN | POLLOUT;
	_client_fd.revents = 0;
	_timestamp = std::time(NULL);
	_log->log_debug( CD_MODULE,
			  "Client Data init.");
}

/**
 * @brief Destructor for `ClientData`, cleaning up resources associated with the client connection.
 *
 * Closes the client's socket file descriptor if it is open, and logs the cleanup process.
 * Catches any exceptions during cleanup to ensure safe and consistent deallocation.
 */
ClientData::~ClientData() {
	close_fd();
}

/**
 * @brief Closes the client's socket file descriptor and marks it as inactive.
 *
 * Attempts to close the client's file descriptor (`fd`) and set `_active` to `false`.
 * Logs the closure process and any issues that arise. If the `fd` is already inactive
 * or invalid, it logs a warning message instead.
 *
 * @note Ensures safe cleanup of client resources, preventing potential resource leaks.
 */
void ClientData::close_fd() {
	try {
		_active = false;
		if (_client_fd.fd) {
			close(_client_fd.fd);
			_log->log_warning( CD_MODULE,
			          "client fd closed and set to inactive.");
		} else {
			_log->log_warning( CD_MODULE,
			          "client fd is not active.");
		}
	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Error Closing Client FD : " << e.what();
		_log->log_error( CD_MODULE,
		          detail.str());
	}
}

/**
 * @brief Retrieves the server's socket handler associated with this client.
 *
 * @return Pointer to the `SocketHandler` instance managing the server connection for this client.
 */
SocketHandler* ClientData::get_server() {
	return (_server);
}


/**
 * @brief Retrieves the client's poll file descriptor structure.
 *
 * Provides access to the `pollfd` structure, which contains the client's file descriptor
 * and event monitoring settings (e.g., `POLLIN` for incoming data).
 *
 * @return A `pollfd` structure containing the client's file descriptor and polling events.
 */
struct pollfd ClientData::get_fd() {
	return (_client_fd);
}

/**
 * @brief Checks if the client connection has exceeded the request timeout.
 *
 * @note This method controls only request related timeout.
 *
 * Compares the current time with the last recorded activity timestamp.
 * If the difference exceeds `TIMEOUT_REQUEST`, marks the client as inactive.
 *
 * @return True if the client connection is still within the timeout period, false otherwise.
 */
bool ClientData::chronos_request() {
	std::time_t now = std::time(NULL);
	if ((now - _timestamp) > TIMEOUT_REQUEST) {
		_alive = false;
	}
	return (_alive);
}

/**
 * @brief Resets the client activity timestamp to the current time.
 *
 * Updates `_timestamp` with the current time, effectively marking recent activity
 * and resetting the timeout period for the client.
 */
void ClientData::chronos_reset() {
	_timestamp = std::time(NULL);
}

/**
 * @brief Checks if the client connection has exceeded the client timeout.
 *
 * @note This method controls timeout using last request timestamp.
 * Similar to `chronos_request()`, but uses a longer timeout threshold (`TIMEOUT_CLIENT`).
 * This is typically used to determine if the client session itself should be terminated.
 *
 * @return True if the client connection is still within the allowed client
 * 		   timeout period, false otherwise.
 */
bool ClientData::chronos_connection() {
	std::time_t now = std::time(NULL);
	if ((now - _timestamp) > TIMEOUT_CLIENT) {
		_alive = false;
	}
	return (_alive);
}

/**
 * @brief Marks the client connection as inactive.
 *
 * Sets `_active` to `false`, indicating that the client connection is no longer active.
 * This can be used in conjunction with timeout checks or when a connection is manually closed.
 */
void ClientData::deactivate() {
	_active = false;
}

/**
 * @brief Checks if the client connection is set to remain active.
 *
 * @return True if the client connection is active, false otherwise.
 */
bool ClientData::is_active() const {
	return (_active);
}

/**
 * @brief Checks if the client was set to kill.
 *
 * @return True if the client is alive, false otherwise.
 */
bool ClientData::is_alive() const {
	return (_alive);
}

/**
 * @brief Sets the client connection as active.
 *
 * Updates the `_active` flag to `true`, indicating that the client connection
 * is currently in use and not eligible for cleanup or timeout handling.
 */
void ClientData::keep_active() {
	_active = true;
}

/**
 * @brief Sets the client life cycle to false.
 *
 * Updates the `_alive` flag to `false`, indicating that the client connection
 * Process should end.
 */
void ClientData::kill_client() {
	_alive = false;
}

s_request& ClientData::client_request() {
	return (_request);
}
