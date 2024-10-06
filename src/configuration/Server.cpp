/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 18:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/06 12:55:05 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Server.hpp"

Server::Server() { }

Server::~Server() { }

void Server::setHost(std::string host) {
    this->_host = host;
}

void Server::setPort(std::string port) {
    this->_port = port;
}

void Server::setServerName(std::string server_name) {
    this->_server_name = server_name;
}

void Server::setErrorPage(std::string error_page) {
    this->_error_page = error_page;
}

void Server::setClientMaxBodySize(std::string client_max_body_size) {
    this->_client_max_body_size = client_max_body_size;
}

void Server::setAutoindex(std::string autoindex) {
    this->_autoindex = autoindex;
}

void Server::setRoot(std::string root) {
    this->_root = root;
}

void Server::setLocations(std::map<std::string, std::string> locations) {
    this->_locations = locations;
}

void Server::setNumLocations(int num_locations) {
    this->_num_locations = num_locations;
}

std::string Server::getHost() const {
    return this->_host;
}

std::string Server::getPort() const {
    return this->_port;
}

std::string Server::getServerName() const {
    return this->_server_name;
}

std::string Server::getErrorPage() const {
    return this->_error_page;
}

std::string Server::getClientMaxBodySize() const {
    return this->_client_max_body_size;
}

std::string Server::getAutoindex() const {
    return this->_autoindex;
}

std::string Server::getRoot() const {
    return this->_root;
}

std::map<std::string, std::string> Server::getLocations() const {
    return this->_locations;
}

int Server::getNumLocations() const {
    return this->_num_locations;
}

bool Server::checkObligatoryParams() {
    if (this->_host.empty() || this->_port.empty() || this->_error_page.empty() || this->_client_max_body_size.empty() || this->_autoindex.empty())
        return false;
    return true;
}

std::ostream& operator<<(std::ostream& os, const Server& server) {
    os << GREEN << "Server name: " << DEF_COLOR << server.getServerName() << std::endl;
    os << GREEN << "Port: " << DEF_COLOR << server.getPort() << std::endl;
    os << GREEN << "Host: " << DEF_COLOR << server.getHost() << std::endl;
    os << GREEN << "Error page: " << DEF_COLOR << server.getErrorPage() << std::endl;
    os << GREEN << "Client max body size: " << DEF_COLOR << server.getClientMaxBodySize() << std::endl;
    os << GREEN << "Autoindex: " << DEF_COLOR << server.getAutoindex() << std::endl;
    os << GREEN << "Root: " << DEF_COLOR << server.getRoot() << std::endl;
    // os << GREEN << "Locations: " << DEF_COLOR << server.getLocations() << std::endl;
    os << GREEN << "Number of locations: " << DEF_COLOR << server.getNumLocations() << std::endl;
    return os;
}
