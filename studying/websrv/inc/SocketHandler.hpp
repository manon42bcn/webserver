/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/14 14:07:40 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _SOCKETHANDLER_HPP_
# define _SOCKETHANDLER_HPP_
# include "webserver.hpp"
# include "http_enum_codes.hpp"
# include "Logger.hpp"
# include <string>

class SocketHandler {
private:
	int                 _socket_fd;
	const ServerConfig& _config;
	const Logger*       _log;
	const std::string   _module;
	std::string         _port_str;

public:
	SocketHandler(int port, const ServerConfig& config, const Logger* logger);
	~SocketHandler();
	int accept_connection();
	int get_socket_fd() const;
	const ServerConfig& get_config() const;
	std::string& get_port();

private:
	bool set_nonblocking(int fd);
};

#endif
