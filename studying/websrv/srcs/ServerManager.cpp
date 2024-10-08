#include "ServerManager.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>

/**
 * @brief Sever Manager Constructor.
 *
 * Create a vector to reserve enough poll_fds.
 */
ServerManager::ServerManager(const std::vector<ServerConfig>& configs) {
	poll_fds.reserve(100);
	for (size_t i = 0; i < configs.size(); ++i) {
		add_server(configs[i].port);
	}
}

/**
 * @brief ServerManager Destructor
 *
 * Closes all sockets, and free reserved memory for SocketHandler instances.
 */
ServerManager::~ServerManager() {
	for (size_t i = 0; i < poll_fds.size(); ++i) {
		close(poll_fds[i].fd);
	}
	for (size_t i = 0; i < servers.size(); ++i) {
		delete servers[i];
	}
}

/**
 * @brief Creates and attach a server, listen in a especific port.
 *
 * @param port Port to listen.
 */
void ServerManager::add_server(int port) {
	SocketHandler* server = new SocketHandler(port);
	server->configure();
	servers.push_back(server);
	add_server_to_poll(server->get_socket_fd());
}

/**
 * @brief Add socket fd to poll_fds vector.
 *
 * @param server_fd socket fd
 */
void ServerManager::add_server_to_poll(int server_fd) {
	struct pollfd pfd;
	pfd.fd = server_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	poll_fds.push_back(pfd);
}

/**
 * @brief Add client fd to poll_fd vector.
 *
 * @param client_fd client fd.
 */
void ServerManager::add_client_to_poll(int client_fd) {
	struct pollfd pfd;
	pfd.fd = client_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	poll_fds.push_back(pfd);
}

/**
 * @brief Start the loop to listen and handle connections and requests.
 */
void ServerManager::run() {
	while (true) {
		int poll_count = poll(&poll_fds[0], poll_fds.size(), -1);

		if (poll_count < 0) {
			std::cerr << "Error en poll()" << std::endl;
			exit(EXIT_FAILURE);
		}

		for (size_t i = 0; i < poll_fds.size(); ++i) {
			if (poll_fds[i].revents & POLLIN) {
				bool is_server = false;

				// Check, the fd belongs to server.
				for (size_t s = 0; s < servers.size(); ++s) {
					if (poll_fds[i].fd == servers[s]->get_socket_fd()) {
						is_server = true;
						break;
					}
				}

				if (is_server) {
					// Accept new connection
					SocketHandler* server = nullptr;
					for (size_t s = 0; s < servers.size(); ++s) {
						if (poll_fds[i].fd == servers[s]->get_socket_fd()) {
							server = servers[s];
							break;
						}
					}

					if (server) {
						int new_client_fd = server->accept_connection();
						if (new_client_fd > 0) {
							add_client_to_poll(new_client_fd);
							std::cout << "Nueva conexión aceptada en el puerto "
							          << server->get_socket_fd() << std::endl;
						}
					}
				} else {
					// Handle request
					HttpRequestHandler request_handler;
					HttpResponseHandler response_handler;

					request_handler.handle_request(poll_fds[i].fd);

					// Aquí manejamos que tipo de respuesta entregar, por ahora, fija.
					response_handler.send_response(poll_fds[i].fd, 200, "Hello, world!");

					// Cerrar la conexión del cliente
					close(poll_fds[i].fd);

					// Eliminar el descriptor de cliente del vector poll_fds
					poll_fds[i] = poll_fds[poll_fds.size() - 1];
					poll_fds.pop_back();
					--i;
				}
			}
		}
	}
}
