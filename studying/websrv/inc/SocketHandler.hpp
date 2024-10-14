/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/14 11:07:12 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKETHANDLER_HPP
#define SOCKETHANDLER_HPP
#include "webserver.hpp"
#include <string>

class SocketHandler {
private:
	int socket_fd;
	const ServerConfig& config;

public:
	SocketHandler(int port, const ServerConfig& config);
	int accept_connection();
	int get_socket_fd() const;
	const ServerConfig& get_config() const;

private:
	void set_nonblocking(int fd);
};

#endif
