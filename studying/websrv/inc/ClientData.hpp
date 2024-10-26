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

//TODO READ ABOUT vsnprintf
class ClientData {
	private:
		const SocketHandler*    _server;
		const Logger*           _log;
	    bool                    _active;
		struct pollfd           _client_fd;
	    std::time_t             _timestamp;
	    std::string             _saludos;
	    size_t                  _poll_index;

	public:
		ClientData(const SocketHandler* server, const Logger* log, int fd);
	    ~ClientData();
	    ClientData& operator=(const ClientData& orig);
	    const SocketHandler* get_server();
	    struct pollfd get_fd();
	    void deactivate();
		void close_fd();
	    void say_hello(std::string saludo);
	    bool keep_alive();
	    std::string& saludo();
	    std::time_t& timer();
	    int crono();
	    void set_index(size_t index);
	    size_t get_index();
};

#endif
