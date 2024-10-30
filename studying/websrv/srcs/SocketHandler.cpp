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
#include <string>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

SocketHandler::SocketHandler(int port, ServerConfig& config, const Logger* logger):
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
	mapping_cgi_locations();
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


bool SocketHandler::get_location(const std::string& path, const std::string& loc_root) {
	std::string saved_key;
	LocationConfig* result = NULL;

	for (std::map<std::string, LocationConfig>::iterator it = _config.locations.begin();
	     it != _config.locations.end(); ++it) {
		const std::string& key = it->first;
		if (starts_with(path, key)) {
			if (key.length() > saved_key.length()) {
				result = &it->second;
				saved_key = key;
			}
		}
	}
	if (result) {
		_log->log(LOG_DEBUG, SH_NAME, result->loc_root + " result + loc " + loc_root);
		return (result->loc_root == loc_root);
	} else {
		return (false);
	}
}


bool SocketHandler::is_cgi_file(const std::string &filename, const std::string& extension) const {
	return (filename.size() >= extension.length()
	        && filename.compare(filename.size() - extension.length(),
        extension.length(), extension) == 0);
}


void SocketHandler::get_cgi_files(const std::string& directory, const std::string& loc_root,
                                  const std::string& extension, std::map<std::string, std::string>& mapped_files) {
	DIR* dir = opendir(directory.c_str());
	if (dir == NULL) {
		_log->log(LOG_WARNING, SH_NAME,
		          "No directory was found.");
		return;
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;

		if (name == "." || name == "..") continue;

		std::string full_path = directory + "/" + name;
		struct stat info;
		if (stat(full_path.c_str(), &info) != 0) {
			_log->log(LOG_WARNING, SH_NAME,
			          full_path + " : is not accessible.");
			continue ;
		}

		if (S_ISDIR(info.st_mode)) {
			get_cgi_files(full_path, loc_root, extension, mapped_files);
		} else if (S_ISREG(info.st_mode) && is_cgi_file(name, extension)) {
			if (get_location(directory, loc_root)) {
				mapped_files.insert(std::make_pair(full_path, name));
			}
		}
	}
	closedir(dir);
}


void SocketHandler::mapping_cgi_locations() {
	_log->log(LOG_DEBUG, SH_NAME,
	          "mapping cgi locations.");
	for (std::map<std::string, LocationConfig>::iterator it = _config.locations.begin(); it != _config.locations.end(); it++) {
		if (it->second.cgi_file) {
			_log->log(LOG_DEBUG, SH_NAME,
			          "Location with CGI activated, mapping for files.");
			get_cgi_files(_config.server_root + it->second.loc_root,
			              it->second.loc_root, ".py", it->second.cgi_locations);
		}
	}
	for (std::map<std::string, LocationConfig>::iterator it = _config.locations.begin(); it != _config.locations.end(); it++) {
		if (it->second.cgi_file) {
			_log->log(LOG_DEBUG, SH_NAME, "LocationROOT " + it->second.loc_root);
			for (std::map<std::string, std::string>::iterator it_p = it->second.cgi_locations.begin(); it_p != it->second.cgi_locations.end(); it_p++) {
				_log->log(LOG_DEBUG, SH_NAME,
				          "path : " + it_p->first + " file " + it_p->second);
			}
		}
	}
}

const char* SocketHandler::SocketCreationError::what(void) const throw() {
	return ("Error Creating Socket.");
}
