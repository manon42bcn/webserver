/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/14 17:19:43 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _SOCKETHANDLER_HPP_
# define _SOCKETHANDLER_HPP_
#include "WebserverException.hpp"
#include "WebserverCache.hpp"
#include "webserver.hpp"
#include "http_enum_codes.hpp"
#include "general_helpers.hpp"
#include "Logger.hpp"
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

# define SH_NAME "SocketHandler"
# define SOCKET_BACKLOG_QUEUE 2048

class SocketHandler {
private:
	int                             _socket_fd;
	ServerConfig&                   _config;
	const Logger*                   _log;
	const std::string               _module;
	std::string                     _port_str;
	WebServerCache<CacheEntry>      _cache;
	WebServerCache<CacheRequest>    _request_cache;

	bool set_nonblocking(int fd);
	static bool is_cgi_file(const std::string& filename, const std::string& extension) ;
	bool belongs_to_location(const std::string& path, const std::string& loc_root);
	void get_cgi_files(const std::string& directory, const std::string& loc_root,
	                   const std::string& extension, std::map<std::string, t_cgi>& mapped_files);
	void mapping_cgi_locations(const std::string& extension);
	void close_socket();
public:
	SocketHandler(int port, ServerConfig& config, const Logger* logger);
	~SocketHandler();
	int accept_connection();
	int get_socket_fd() const;
	const ServerConfig& get_config() const;
	std::string get_port() const;
	WebServerCache<CacheEntry>&   get_cache();
	WebServerCache<CacheRequest>& get_request_cache();
};

#endif
