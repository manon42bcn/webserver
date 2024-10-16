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

#ifndef SOCKETHANDLER_HPP
#define SOCKETHANDLER_HPP
#include "webserver.hpp"
#include "http_enum_codes.hpp"
#include <string>

class SocketHandler {
private:
	int _socket_fd;
	const ServerConfig& _config;

public:
	SocketHandler(int port, const ServerConfig& config);
	int accept_connection();
	int get_socket_fd() const;
	const ServerConfig& get_config() const;

private:
	void set_nonblocking(int fd);
};

#endif
