/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientData.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 08:43:27 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/07 17:15:26 by mporras-         ###   ########.fr       */
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
