///* ************************************************************************** */
///*                                                                            */
///*                                                        :::      ::::::::   */
///*   main.cpp                                           :+:      :+:    :+:   */
///*                                                    +:+ +:+         +:+     */
///*   By: mac <marvin@42.fr>                         +#+  +:+       +#+        */
///*                                                +#+#+#+#+#+   +#+           */
///*   Created: 2024/09/24 23:49:21 by mac               #+#    #+#             */
///*   Updated: 2024/09/24 23:49:24 by mac              ###   ########.fr       */
///*                                                                            */
///* ************************************************************************** */
//
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <poll.h>

#include "webserver.hpp"
//#include "HttpRequestHandler.hpp"
//#include "HttpResponseHandler.hpp"
#include "ServerManager.hpp"
//#include "SocketHandler.hpp"

int main() {
	std::vector<ServerConfig> configs;

	// Configuración del servidor en el puerto 8080
	ServerConfig server1;
	server1.port = 8080;
	server1.server_name = "localhost";
	server1.document_root = "/var/www/html";
	server1.error_pages[404] = "/404.html";
	server1.locations = std::map<std::string, std::string>();
	server1.default_pages.push_back("index.html");
	configs.push_back(server1);

	// Configuración del servidor en el puerto 9090
	ServerConfig server2;
	server2.port = 9090;
	server2.server_name = "localhost";
	server2.document_root = "/var/www/site";
	server2.error_pages[404] = "/404.html";
	server2.locations = std::map<std::string, std::string>();
	server2.default_pages.push_back("index.html");
	configs.push_back(server2);
	ServerManager server_manager(configs);

	// Iniciar el ciclo de eventos
	server_manager.run();

	return 0;
}
