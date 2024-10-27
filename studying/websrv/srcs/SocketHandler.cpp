/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/14 14:07:40 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SocketHandler.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

/**
 * @brief Constructor for the SocketHandler class.
 *
 * This constructor creates and configures a new socket for the server. It binds the
 * socket to the specified port and puts it in listening mode. If any error occurs
 * during the socket creation, binding, or listening process, a fatal log is generated,
 * and the program may exit (depending on the logger behavior).
 *
 * @param port The port on which the server will listen for incoming connections.
 * @param config The server configuration.
 * @param logger A pointer to the Logger instance for logging server activity.
 *
 * @note The constructor will attempt to clean up resources (such as closing the socket)
 * in the event of an error.
 *
 * @exception None directly thrown, but the logger may exit the program on fatal errors.
 */

SocketHandler::SocketHandler(int port, const ServerConfig& config, const Logger* logger):
		_socket_fd(-1),
        _config(config),
        _log(logger),
        _module("SocketHandler") {
	if (_log == NULL) {
		throw Logger::NoLoggerPointer();
	}
	_log->log(LOG_DEBUG, _module, "Creating Sockets.");
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd < 0)
		_log->fatal_log(_module, "Error creating socket.");

	_log->log(LOG_DEBUG, _module, "Configure server address.");
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
	_port_str = int_to_string(port);
	_log->log(LOG_DEBUG, _module, "Linking Socket.");
	if (bind(_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		_log->fatal_log(_module, "Error linking Socket.");
	_log->log(LOG_DEBUG, _module, "Socket to listening mode.");
	if (listen(_socket_fd, 10) < 0)
		_log->fatal_log(_module, "Error at listening process.");
	set_nonblocking(_socket_fd);
	_log->log(LOG_INFO, _module,
			  "Server listening. Port: " + int_to_string(port));
}

/**
 * @brief Destructor for the SocketHandler class.
 *
 * This destructor ensures that the socket file descriptor is properly closed when
 * the `SocketHandler` instance is destroyed. This prevents file descriptor leaks
 * and ensures proper cleanup of system resources associated with the socket.
 *
 * @note This destructor is automatically called when the object goes out of scope
 * or when it is explicitly deleted, so manual cleanup of the socket is not required.
 *
 * @param None
 * @return None
 */
SocketHandler::~SocketHandler() {
	if (_socket_fd >= 0) {
		close(_socket_fd);
		_log->log(LOG_DEBUG, _module, "Socket closed.");
	}
}

std::string& SocketHandler::get_port() {
	return (this->_port_str);
}

int SocketHandler::accept_connection() {
	_log->log(LOG_DEBUG, _module,"Accepting Connection.");
	int client_fd = accept(_socket_fd, NULL, NULL);
	if (client_fd < 0) {
		_log->log(LOG_ERROR, _module,"Error accepting connection.");
	} else {
		set_nonblocking(client_fd);  // Asegurarse de que el socket del cliente sea no bloqueante
		_log->log(LOG_INFO, _module,"Connection Accepted.");
	}
	return client_fd;
}

/**
 * @brief Returns Socket FD
 *
 * @return Socket FD
 */
int SocketHandler::get_socket_fd() const {
	return (_socket_fd);
}

/**
 * @brief Returns Server Config Reference
 *
 * @return ServerConfig Reference
 */
const ServerConfig& SocketHandler::get_config() const {
	return (_config);
}

/**
 * @brief Sets a socket file descriptor to non-blocking mode.
 *
 * This method modifies the file descriptor flags for the given socket to ensure that
 * it operates in non-blocking mode, allowing the server to handle multiple client connections
 * without blocking.
 *
 * @details
 * - If an error occurs while getting or setting the socket flags, an error log is generated.
 * - The method returns a boolean value indicating whether the operation was successful or not.
 *
 * @param fd The file descriptor of the socket to be set to non-blocking mode.
 * @return bool True if the operation was successful, false if there was an error.
 */
bool SocketHandler::set_nonblocking(int fd) {
	_log->log(LOG_DEBUG, _module, "Set connection as nonblocking.");
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		_log->log(LOG_ERROR, _module, "Error getting socket flags.");
		return (false);
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		_log->log(LOG_ERROR, _module, "Error setting socket as nonblocking.");
		return (false);
	}
	_log->log(LOG_INFO, _module, "Socket set as nonblocking.");
	return (true);
}
