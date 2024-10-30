/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/30 14:05:44 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _SOCKETHANDLER_HPP_
# define _SOCKETHANDLER_HPP_
# include "webserver.hpp"
# include "http_enum_codes.hpp"
# include "Logger.hpp"
# include <string>

# define SH_NAME "SocketHandler"

class SocketHandler {
private:
	int                 _socket_fd;
	ServerConfig&       _config;
	const Logger*       _log;
	const std::string   _module;
	std::string         _port_str;
	bool				_cgi_locations;

	bool set_nonblocking(int fd);
	static bool is_cgi_file(const std::string& filename, const std::string& extension) ;
	bool belongs_to_location(const std::string& path, const std::string& loc_root);
	void get_cgi_files(const std::string& directory, const std::string& loc_root,
	                   const std::string& extension, std::map<std::string, t_cgi>& mapped_files);
	void mapping_cgi_locations();
public:
	SocketHandler(int port, ServerConfig& config, const Logger* logger);
	~SocketHandler();
	int accept_connection();
	int get_socket_fd() const;
	const ServerConfig& get_config() const;
	const bool& is_cgi_server() const;
	std::string& get_port();
	class SocketCreationError : public std::exception {
	public:
		virtual const char *what() const throw();
	};
};

#endif
