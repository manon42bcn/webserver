/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientData.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 08:43:27 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/26 22:04:29 by mporras-         ###   ########.fr       */
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
# define TIMEOUT_REQUEST 10
# define TIMEOUT_CLIENT 10

/**
 * @class ClientData
 * @brief Manages individual client connections in the web server, tracking their state and activity.
 *
 * The `ClientData` class encapsulates client connection data and provides
 * methods to monitor and control the client's connection state, such as
 * handling timeouts, deactivation, and status checks. Each instance is
 * associated with a specific client connection, providing an interface
 * for managing socket file descriptors, connection timestamps, and logging.
 */
class ClientData {
	private:
		SocketHandler*          _server;
		const Logger*           _log;
	    bool                    _active;
		bool                    _alive;
		struct pollfd           _client_fd;
	    std::time_t             _timestamp;
		s_request               _request;
		short                   _state;

	public:
		ClientData(SocketHandler* server, const Logger* log, int fd);
	    ~ClientData();
		void close_fd();
		SocketHandler* get_server();
		struct pollfd get_fd();
		bool chronos_request();
		void chronos_reset();
		bool chronos_connection();
		void deactivate();
		void kill_client();
		bool is_alive() const;
		bool is_active() const;
		void keep_active();
		s_request& client_request();
		void set_state(short state);
		short get_state() const;
};

#endif
