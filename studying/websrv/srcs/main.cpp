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

#include "SocketHandler.hpp"
#include "HttpRequestHandler.hpp"
#include "HttpResponseHandler.hpp"
#include "ServerManager.hpp"

int main() {
	ServerManager server_manager;

	// Agregar servidores en los puertos 8080 y 9090
	server_manager.add_server(8080);
	server_manager.add_server(9090);

	// Iniciar el ciclo de eventos
	server_manager.run();

	return 0;
}
