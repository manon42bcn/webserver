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

# define CD_MODULE "ClientData"

class ClientData {
	private:
		SocketHandler*      _server;
		const Logger*       _log;
	    bool                _active;
		struct pollfd       _client_fd;

	public:
		ClientData(SocketHandler* server, const Logger* log, int fd);
	    ~ClientData();
	    ClientData& operator=(const ClientData& orig);
	    SocketHandler* get_server();
	    struct pollfd& get_fd();
	    void deactivate();
		void close_fd();
};

#endif
