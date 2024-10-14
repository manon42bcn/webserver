/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/14 11:07:12 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SocketHandler.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

/**
 * @brief Constructor de SocketHandler que acepta configuración de servidor.
 *
 * @param port Número del puerto en el que el servidor escuchará.
 * @param config Configuración asociada a este servidor.
 */
SocketHandler::SocketHandler(int port, const ServerConfig& config)
		:socket_fd(-1), config(config) {
	// Crear el socket
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		std::cerr << "Error al crear el socket" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Configurar la dirección del servidor
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	// Enlazar el socket
	if (bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		std::cerr << "Error al enlazar el socket" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Poner el socket en modo escucha
	if (listen(socket_fd, 10) < 0) {
		std::cerr << "Error al escuchar en el socket" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Configurar el socket como no bloqueante
	set_nonblocking(socket_fd);

	std::cout << "Servidor configurado y escuchando en el puerto " << port << std::endl;
}

/**
 * @brief Acepta una nueva conexión.
 *
 * @return Descriptor del socket del cliente.
 */
int SocketHandler::accept_connection() {
	int client_fd = accept(socket_fd, NULL, NULL);
	if (client_fd < 0) {
		std::cerr << "Error al aceptar la conexión" << std::endl;
	} else {
		set_nonblocking(client_fd);  // Asegurarse de que el socket del cliente sea no bloqueante
	}
	return client_fd;
}

/**
 * @brief Devuelve el descriptor del socket.
 *
 * @return int Descriptor del socket.
 */
int SocketHandler::get_socket_fd() const {
	return socket_fd;
}

/**
 * @brief Devuelve la configuración del servidor.
 *
 * @return ServerConfig Estructura con la configuración del servidor.
 */
const ServerConfig& SocketHandler::get_config() const {
	return config;
}

/**
 * @brief Configura el socket como no bloqueante.
 *
 * @param fd Descriptor del socket.
 */
void SocketHandler::set_nonblocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		std::cerr << "Error al obtener las banderas del socket" << std::endl;
		exit(EXIT_FAILURE);
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		std::cerr << "Error al configurar el socket como no bloqueante" << std::endl;
		exit(EXIT_FAILURE);
	}
}
