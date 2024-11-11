/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/30 10:37:47 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/11 02:20:11 by mporras-         ###   ########.fr       */
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

void signal_handler(int sig) {
	if (running_server != NULL) {
		std::cout << "\nReceived signal " << sig << ". Shutting down server..." << std::endl;
		running_server->turn_off_server();
	}
}

int main(int argc, char **argv) {

	(void)argc;
	(void)argv;
	Logger logger(LOG_ERROR, true);
	std::string base_path = get_server_root();
	std::vector<ServerConfig> configs;
	std::map<std::string, LocationConfig> locations;


	// Datos de prueba
	std::vector<std::string> default_pages;
	default_pages.push_back("index.html");
	default_pages.push_back("home.html");
	std::map<int, std::string> error_pages;
	error_pages[404] = "404.html";

	// Insertar datos en el vector
	locations.insert(std::make_pair("/", LocationConfig("", ACCESS_READ, default_pages, TEMPLATE, error_pages)));
	locations.insert(std::make_pair("/home", LocationConfig("/home", ACCESS_DELETE, default_pages, TEMPLATE, error_pages)));
	locations.insert(std::make_pair("/home/other/path", LocationConfig("/home/other/path", ACCESS_WRITE, default_pages, TEMPLATE, error_pages)));
	locations.insert(std::make_pair("/admin", LocationConfig("/admin", ACCESS_FORBIDDEN, default_pages, LITERAL, error_pages)));
	locations.insert(std::make_pair("/public", LocationConfig("/public", ACCESS_READ, default_pages, LITERAL, error_pages)));

	ServerConfig server1;
	server1.port = 8080;
	server1.server_name = "localhost";
	server1.server_root = base_path + "data";
	server1.error_pages[404] = "/404.html";
	server1.locations = locations;
	server1.default_pages.push_back("index.html");
	server1.ws_root = base_path + "/data";
	server1.ws_errors_root = base_path + "default_error_pages";
	configs.push_back(server1);

	ServerConfig server2;
	server2.port = 9090;
	server2.server_name = "localhost";
	server2.server_root =  base_path + "data/9090";
	server2.error_pages[404] = "/404.html";
	server2.locations = locations;
	server2.default_pages.push_back("index.html");
	server2.default_pages.push_back("home.html");
	server2.default_pages.push_back("index.htm");
	server2.ws_root = base_path + "/data";
	server2.ws_errors_root = base_path + "default_error_pages";
	configs.push_back(server2);
	WebServerCache cache(200);
	
	try {
		ServerManager server_manager(configs, &logger, &cache);
		running_server = &server_manager;
		signal(SIGINT, signal_handler);
		signal(SIGTERM, signal_handler);
		signal(SIGTSTP, signal_handler);
		server_manager.run();
	} catch (const std::exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		if (running_server != NULL) {
			running_server->turn_off_server();
		}
	}
	
	return 0;
}
