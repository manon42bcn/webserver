/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientData.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mac <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/19 19:39:57 by mac               #+#    #+#             */
/*   Updated: 2024/10/19 19:40:00 by mac              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CLIENT_DATA_HPP_
#define _CLIENT_DATA_HPP_

#include "SocketHandler.hpp"
#include "Logger.hpp"
#include <poll.h>
#include <unistd.h>
#include <ctime>

# define CD_MODULE "ClientData"
# define TIMEOUT_LIMIT 10

//TODO READ ABOUT vsnprintf
class ClientData {
	private:
		const SocketHandler*    _server;
		const Logger*           _log;
	    bool                    _active;
		struct pollfd           _client_fd;
	    std::time_t             _timestamp;

	public:
		ClientData(const SocketHandler* server, const Logger* log, int fd);
	    ~ClientData();
	    ClientData& operator=(const ClientData& orig);
	    const SocketHandler* get_server();
	    struct pollfd get_fd();
	    void deactivate();
		void keep_active();
		void close_fd();
	    bool chronos();
	    void chronos_reset();
		bool keep_alive();
};

#endif
