/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mac <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 23:49:21 by mac               #+#    #+#             */
/*   Updated: 2024/09/24 23:49:24 by mac              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>

#define PORT 8080
#define MAX_CLIENTS 100

int set_non_blocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		std::cerr << "Error al obtener flags del socket" << std::endl;
		exit(EXIT_FAILURE);
	}
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) == -1) {
		std::cerr << "Error al configurar el socket como no bloqueante" << std::endl;
		exit(EXIT_FAILURE);
	}
	return 0;
}

int main() {
	int server_fd, new_socket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	// Crear el socket
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		std::cerr << "Error al crear el socket" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Configurar la dirección y el puerto
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Vincular el socket al puerto
	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		std::cerr << "Error en bind()" << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	// Escuchar en el puerto
	if (listen(server_fd, 10) < 0) {
		std::cerr << "Error en listen()" << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	// Configurar el socket del servidor como no bloqueante
	set_non_blocking(server_fd);

	// Crear estructura poll
	struct pollfd fds[MAX_CLIENTS];
	fds[0].fd = server_fd;  // Descriptor del socket del servidor
	fds[0].events = POLLIN; // Interesado en el evento de lectura
	int nfds = 1;           // Número de file descriptors activos

	std::cout << "Servidor escuchando en el puerto " << PORT << std::endl;

	while (true) {
		// Llamada a poll() para verificar actividad en los sockets
		int poll_count = poll(fds, nfds, -1);

		if (poll_count == -1) {
			std::cerr << "Error en poll()" << std::endl;
			close(server_fd);
			exit(EXIT_FAILURE);
		}

		// Verificar si el socket del servidor tiene una nueva conexión
		if (fds[0].revents & POLLIN) {
			if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) >= 0) {
				// Configurar el nuevo socket como no bloqueante
				set_non_blocking(new_socket);
				std::cout << "Nueva conexión establecida en el socket " << new_socket << std::endl;

				// Agregar el nuevo socket al array de poll
				fds[nfds].fd = new_socket;
				fds[nfds].events = POLLIN;
				nfds++;
			}
		}

		// Verificar actividad en los sockets de los clientes
		for (int i = 1; i < nfds; i++) {
			if (fds[i].revents & POLLIN) {
				char buffer[1024] = {0};
				int valread = read(fds[i].fd, buffer, 1024);
				if (valread > 0) {
					std::cout << "Mensaje recibido: " << buffer << std::endl;

					// Enviar una respuesta simple
					std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
					send(fds[i].fd, response.c_str(), response.length(), 0);
				} else if (valread == 0) {
					// El cliente cerró la conexión
					std::cout << "Conexión cerrada en el socket " << fds[i].fd << std::endl;
					close(fds[i].fd);
					// Eliminar el socket del array de poll
					fds[i] = fds[nfds - 1];
					nfds--;
				}
			}
		}
	}

	close(server_fd);
	return 0;
}
