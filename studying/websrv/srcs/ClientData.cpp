/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientData.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 08:43:27 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/07 17:16:54 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "ClientData.hpp"

ClientData::ClientData(const SocketHandler* server, const Logger* log, int fd):
	_server(server),
	_log(log),
    _active(false),
	_client_fd() {
	if (_log == NULL){
		throw Logger::NoLoggerPointer();
	}
	_client_fd.fd = fd;
	_client_fd.events = POLLIN;
	_timestamp = std::time(NULL);
}

ClientData::~ClientData() {
	if (_client_fd.fd >= 0) {
		close(_client_fd.fd);
	}
	_log->log(LOG_DEBUG, CD_MODULE,
	          "Client Data Resources clean up.");
	_log = NULL;
}

bool ClientData::chronos() {
	std::time_t now = std::time(NULL);
	if (now - _timestamp > TIMEOUT_LIMIT) {
		_active = false;
	}
	return (_active);
}

void ClientData::chronos_reset() {
	_timestamp = std::time(NULL);
}

ClientData& ClientData::operator=(const ClientData& orig)
{
	if(this == &orig)
		return (*this);
	this->_server = orig._server;
	this->_log = orig._log;
	this->_active = orig._active;
	this->_client_fd = orig._client_fd;
	return (*this);
}

const SocketHandler* ClientData::get_server() {
	return (_server);
}

struct pollfd ClientData::get_fd() {
	return (_client_fd);
}

void ClientData::deactivate() {
	_active = false;
}

void ClientData::close_fd() {
	if (_client_fd.fd) {
		close(_client_fd.fd);
		_log->log(LOG_WARNING, CD_MODULE, "client fd closed and set to inactive.");
	} else {
		_log->log(LOG_WARNING, CD_MODULE, "client fd is not active.");
	}
	_active = false;
}

bool ClientData::keep_alive() {
	return (_active);
}

void ClientData::keep_active() {
	_active = true;
}
