/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/14 13:50:25 by mporras-         ###   ########.fr       */
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

#include "webserver.hpp"
//#include "HttpRequestHandler.hpp"
//#include "HttpResponseHandler.hpp"
#include "ServerManager.hpp"
//#include "SocketHandler.hpp"

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

	// Imprimir las rutas configuradas en 'locations'
	std::cout << "  Locations: " << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = config.locations.begin(); it != config.locations.end(); ++it) {
		std::cout << "    Path: " << it->first << ", Directory: " << it->second << std::endl;
	}

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

std::string html_codes(int code) {
	static std::map<int, std::string> html_codes;

	if (html_codes.empty()) {
		html_codes[200] = "OK";
		html_codes[403] = "Forbidden";
		html_codes[400] = "Bad Request";
		html_codes[500] = "Internal Server Error";
		html_codes[503] = "Service Unavailable";
		html_codes[502] = "Bad Gateway";
		html_codes[504] = "Gateway Timeout";
		html_codes[401] = "Unauthorized";
		html_codes[405] = "Method Not Allowed";
		html_codes[416] = "Range Not Satisfiable";
		html_codes[413] = "Payload Too Large";
	}
	std::map<int, std::string >::const_iterator it = html_codes.find(code);
	//	TODO: This behaviour should be controlled. Each error that can be handle should have its code
	if (it == html_codes.end())
		return ("NO INFO");
	return (it->second);
}

int main() {
	std::vector<ServerConfig> configs;


	ServerConfig server1;
	server1.port = 8080;
	server1.server_name = "localhost";
	server1.server_root = "/Users/mac/Documents/Cursus/webserver/studying/websrv/data";
	server1.error_pages[404] = "/404.html";
	server1.locations = std::map<std::string, std::string>();
	server1.default_pages.push_back("index.html");
	server1.ws_root = "/Users/mac/Documents/Cursus/webserver/studying/websrv/data";
	server1.ws_errors_root = "/Users/mac/Documents/Cursus/webserver/studying/websrv/default_error_pages";
	configs.push_back(server1);

	ServerConfig server2;
	server2.port = 9090;
	server2.server_name = "localhost";
	server2.server_root = "/Users/mac/Documents/Cursus/webserver/studying/websrv/data/9090";
	server2.error_pages[404] = "/404.html";
	server2.locations = std::map<std::string, std::string>();
	server2.default_pages.push_back("index.html");
	server2.default_pages.push_back("home.html");
	server2.default_pages.push_back("index.htm");
	server2.ws_root = "/Users/mac/Documents/Cursus/webserver/studying/websrv/data";
	server2.ws_errors_root = "/Users/mac/Documents/Cursus/webserver/studying/websrv/default_error_pages";
	configs.push_back(server2);
	ServerManager server_manager(configs);

	// Iniciar el ciclo de eventos
	server_manager.run();

	return 0;
}
