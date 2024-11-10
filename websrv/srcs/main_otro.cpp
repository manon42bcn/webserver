/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 10:37:47 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/10 03:31:37 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <poll.h>
#include <sys/stat.h>
#include <map>

#include "webserver.hpp"
//#include "HttpRequestHandler.hpp"
//#include "HttpResponseHandler.hpp"
#include "ServerManager.hpp"
//#include "SocketHandler.hpp"
#include <cstdlib>
#include <signal.h>
#include "WebserverCache.hpp"

void print_server_config(const ServerConfig& config, std::string location) {
	std::cout << "FROM: " << location << std::endl;
	std::cout << "Server Configuration:" << std::endl;
	std::cout << "  Port: " << config.port << std::endl;
	std::cout << "  Server Name: " << config.server_name << std::endl;
	std::cout << "  Document Root: " << config.server_root << std::endl;

	// Imprimir las páginas de error personalizadas
	std::cout << "  Error Pages: " << std::endl;
	for (std::map<int, std::string>::const_iterator it = config.error_pages.begin(); it != config.error_pages.end(); ++it) {
		std::cout << "    Error " << it->first << ": " << it->second << std::endl;
	}

	// Imprimir las páginas por defecto
	std::cout << "  Default Pages: " << std::endl;
	for (std::vector<std::string>::const_iterator it = config.default_pages.begin(); it != config.default_pages.end(); ++it) {
		std::cout << "    " << *it << std::endl;
	}

//	// Imprimir las rutas configuradas en 'locations'
//	std::cout << "  Locations: " << std::endl;
//	for (std::map<std::string, std::string>::const_iterator it = config.locations.begin(); it != config.locations.end(); ++it) {
//		std::cout << "    Path: " << it->first << ", Directory: " << it->second << std::endl;
//	}

	std::cout << "------------------"  << std::endl;
}

void print_vector_config(const std::vector<ServerConfig> &config, std::string location)
{
	for (size_t i = 0; i < config.size(); i++)
		print_server_config(config[i], location);
	exit(0);
}

bool is_dir(std::string ruta)
{
	struct stat s;

	if (stat(ruta.c_str(), &s) == 0) {
		return S_ISDIR(s.st_mode);
	}
	return false;
}

bool is_file(std::string ruta) {
	struct stat s;

	if (stat(ruta.c_str(), &s) == 0) {
		return S_ISREG(s.st_mode);
	}
	return false;
}

/**
 * @brief Converts an integer to a string.
 *
 * @param number The integer to convert.
 * @return A string representation of the integer.
 */
std::string int_to_string(int number) {
	std::stringstream ss;
	ss << number;
	return ss.str();
}

// WIP: Starts_with to compare a given path with each locations key...
bool starts_with(const std::string& str, const std::string& prefix) {
	if (str.size() < prefix.size()) {
		return (false);
	}
	return (str.compare(0, prefix.size(), prefix) == 0);
}

ServerManager* running_server = NULL;

void signal_handler(int sig){
	(void)sig;
	running_server->turn_off_server();
}

int main(int argc, char **argv) {

	Logger logger(LOG_ERROR, false);
	std::string base_path = get_server_root();
	std::vector<ServerConfig> configs;
	std::map<std::string, LocationConfig> locations;

	if (!check_args(argc, argv))
		exit(1);
	configs = parse_file(argv[1], &logger);


	WebServerCache cache(200);
	try {
		ServerManager server_manager(configs, &logger, &cache);
		signal(SIGINT, signal_handler);
		running_server = &server_manager;
		server_manager.run();
	} catch (Logger::NoLoggerPointer& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
	} catch (WebServerException& e){
		std::cerr << "ERROR: " << e.what() << std::endl;
	} catch (std::exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
	}
	return 0;
}