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


SocketHandler::SocketHandler(int port, const ServerConfig& config, const Logger* logger):
		_socket_fd(-1),
        _config(config),
        _log(logger){
	if (_log == NULL) {
		throw Logger::NoLoggerPointer();
	}
	_log->log(LOG_DEBUG, SH_NAME,
	          "Creating Sockets.");
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd < 0) {
		throw SocketHandler::SocketCreationError();
	}

	_log->log(LOG_DEBUG, SH_NAME, "Configure server address.");
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
	_port_str = int_to_string(port);
	_log->log(LOG_DEBUG, SH_NAME, "Linking Socket.");
	if (bind(_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		_log->fatal_log(SH_NAME, "Error linking Socket.");
	_log->log(LOG_DEBUG, SH_NAME, "Socket to listening mode.");
	if (listen(_socket_fd, 10) < 0)
		_log->fatal_log(SH_NAME, "Error at listening process.");
	set_nonblocking(_socket_fd);
	_log->log(LOG_INFO, SH_NAME,
			  "Server listening. Port: " + int_to_string(port));
}

SocketHandler::~SocketHandler() {
	if (_socket_fd >= 0) {
		close(_socket_fd);
		_log->log(LOG_DEBUG, SH_NAME,
		          "Socket closed.");
	}
	_log->log(LOG_DEBUG, SH_NAME,
	          "SockedHandler resources clean up.");
	_log = NULL;
}

std::string& SocketHandler::get_port(){
	return (this->_port_str);
}

int SocketHandler::accept_connection() {
	_log->log(LOG_DEBUG, SH_NAME,"Accepting Connection.");
	int client_fd = accept(_socket_fd, NULL, NULL);
	if (client_fd < 0) {
		_log->log(LOG_ERROR, SH_NAME,"Error accepting connection.");
	} else {
		set_nonblocking(client_fd);  // Asegurarse de que el socket del cliente sea no bloqueante
		_log->log(LOG_INFO, SH_NAME,"Connection Accepted.");
	}
	return client_fd;
}


int SocketHandler::get_socket_fd() const {
	return (_socket_fd);
}

const ServerConfig& SocketHandler::get_config() const {
	return (_config);
}

bool SocketHandler::set_nonblocking(int fd) {
	_log->log(LOG_DEBUG, SH_NAME, "Set connection as nonblocking.");
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		_log->log(LOG_ERROR, SH_NAME, "Error getting socket flags.");
		return (false);
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		_log->log(LOG_ERROR, SH_NAME, "Error setting socket as nonblocking.");
		return (false);
	}
	_log->log(LOG_INFO, SH_NAME, "Socket set as nonblocking.");
	return (true);
}

const char* SocketHandler::SocketCreationError::what(void) const throw() {
	return ("Error Creating Socket.");
}
