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
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>

#define PORT1 8080
#define PORT2 9090
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

int set_nonblocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) return -1;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int create_server_socket(int port) {
	int server_fd;
	struct sockaddr_in address;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		std::cerr << "Error al crear el socket." << std::endl;
		exit(EXIT_FAILURE);
	}

	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		std::cerr << "Error al configurar el socket." << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		std::cerr << "Error al hacer bind en el puerto " << port << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 3) < 0) {
		std::cerr << "Error al escuchar en el puerto " << port << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	std::cout << "Servidor escuchando en el puerto " << port << std::endl;
	return server_fd;
}

int main() {
	int server_fd1 = create_server_socket(PORT1);
	int server_fd2 = create_server_socket(PORT2);

	set_nonblocking(server_fd1);
	set_nonblocking(server_fd2);

	struct pollfd poll_fds[MAX_CLIENTS];
	int nfds = 2;

	poll_fds[0].fd = server_fd1;
	poll_fds[0].events = POLLIN;

	poll_fds[1].fd = server_fd2;
	poll_fds[1].events = POLLIN;

	char buffer[BUFFER_SIZE];

	while (true) {
		int poll_count = poll(poll_fds, nfds, -1);

		if (poll_count < 0) {
			std::cerr << "Error en poll()." << std::endl;
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i < nfds; ++i) {
			if (poll_fds[i].revents & POLLIN) {
				if (poll_fds[i].fd == server_fd1 || poll_fds[i].fd == server_fd2) {
					int new_socket;
					struct sockaddr_in address;
					socklen_t addrlen = sizeof(address);

					new_socket = accept(poll_fds[i].fd, (struct sockaddr *)&address, &addrlen);
					if (new_socket < 0) {
						std::cerr << "Error al aceptar conexión." << std::endl;
						continue;
					}

					std::cout << "Nueva conexión aceptada en el puerto "
					          << (poll_fds[i].fd == server_fd1 ? PORT1 : PORT2)
					          << std::endl;

					set_nonblocking(new_socket);
					poll_fds[nfds].fd = new_socket;
					poll_fds[nfds].events = POLLIN;
					nfds++;
				} else {
					int valread = read(poll_fds[i].fd, buffer, BUFFER_SIZE);
					if (valread == 0) {
						std::cout << "Cliente desconectado." << std::endl;
						close(poll_fds[i].fd);
						poll_fds[i] = poll_fds[nfds - 1];
						nfds--;
					} else if (valread > 0) {
						buffer[valread] = '\0';
						std::cout << "Mensaje recibido: " << buffer << std::endl;
						std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
						send(poll_fds[i].fd, response.c_str(), response.length(), 0);
					}
				}
			}
		}
	}

	close(server_fd1);
	close(server_fd2);
	return 0;
}
