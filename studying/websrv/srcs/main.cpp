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
#include "SocketHandler.hpp"
#include "HttpRequestHandler.hpp"
#include <poll.h>

int main() {
	SocketHandler server_socket(8080);
	server_socket.configure();

	HttpRequestHandler request_handler;

	struct pollfd poll_fds[100];
	int nfds = 1;

	poll_fds[0].fd = server_socket.get_socket_fd();
	poll_fds[0].events = POLLIN;

	while (true) {
		int poll_count = poll(poll_fds, nfds, -1);
		if (poll_count < 0) {
			std::cerr << "Error en poll()" << std::endl;
			exit(EXIT_FAILURE);
		}

		if (poll_fds[0].revents & POLLIN) {
			int new_client_fd = server_socket.accept_connection();
			if (new_client_fd > 0) {
				poll_fds[nfds].fd = new_client_fd;
				poll_fds[nfds].events = POLLIN;
				nfds++;
			}
		}

		for (int i = 1; i < nfds; ++i) {
			if (poll_fds[i].revents & POLLIN) {
				request_handler.handle_request(poll_fds[i].fd);
				close(poll_fds[i].fd);
				poll_fds[i] = poll_fds[nfds - 1];
				nfds--;
			}
		}
	}

	server_socket.close_socket();
	return 0;
}
