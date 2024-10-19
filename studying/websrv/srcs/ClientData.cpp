/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientData.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mac <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/19 19:40:20 by mac               #+#    #+#             */
/*   Updated: 2024/10/19 19:40:23 by mac              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientData.hpp"

ClientData::ClientData(SocketHandler *server, const Logger* log, int fd):
	_server(server),
	_log(log),
    _active(true),
	_client_fd() {
	if (_log == NULL){
		throw Logger::NoLoggerPointer();
	}
	_client_fd.fd = fd;
	_client_fd.events = POLLIN;
}

ClientData::~ClientData() {}

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

SocketHandler* ClientData::get_server() {
	return (_server);
}

struct pollfd& ClientData::get_fd() {
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
