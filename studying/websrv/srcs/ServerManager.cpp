#include "ServerManager.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>

/**
 * @brief Constructor of ServerManager that receives server configurations.
 *
 * @param configs Vector with the configurations of the servers.
 */
ServerManager::ServerManager(const std::vector<ServerConfig>& configs) {
	poll_fds.reserve(100);  // Reserve space for poll descriptors
	// Create servers based on the configurations
	for (size_t i = 0; i < configs.size(); ++i) {
		add_server(configs[i].port, configs[i]);
	}
	print_vector_config(configs, "HERE WE ARE");
}

/**
 * @brief Adds a new server with the specified port.
 *
 * @param port Port number where the server will listen.
 * @param config Configuration of the server.
 */
void ServerManager::add_server(int port, const ServerConfig& config) {
	SocketHandler* server = new SocketHandler(port, config);
	servers.push_back(server);
	add_server_to_poll(server->get_socket_fd());
}

/**
 * @brief Adds a server socket to the poll set to monitor its activity.
 *
 * @param server_fd File descriptor of the server socket.
 */
void ServerManager::add_server_to_poll(int server_fd) {
	struct pollfd pfd;
	pfd.fd = server_fd;
	pfd.events = POLLIN;  // Monitor for incoming connections
	pfd.revents = 0;
	poll_fds.push_back(pfd);
}

/**
 * @brief Adds a new client to the poll set and associates it with a server configuration.
 *
 * @param client_fd File descriptor of the client.
 * @param config Server configuration associated with this client.
 */
void ServerManager::add_client_to_poll(int client_fd) {
	struct pollfd pfd;
	pfd.fd = client_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	poll_fds.push_back(pfd);
}

/**
 * @brief Starts the event loop to handle incoming connections and requests.
 */
void ServerManager::run() {
	while (true) {
		int poll_count = poll(&poll_fds[0], poll_fds.size(), -1);

		if (poll_count < 0) {
			std::cerr << "Error in poll()" << std::endl;
			exit(EXIT_FAILURE);
		}

		for (size_t i = 0; i < poll_fds.size(); ++i) {
			if (poll_fds[i].revents & POLLIN) {
				bool is_server = false;

				SocketHandler* server = nullptr;
				for (size_t s = 0; s < servers.size(); ++s) {
					if (poll_fds[i].fd == servers[s]->get_socket_fd()) {
						is_server = true;
						server = servers[s];
						break;
					}
				}

				if (is_server) {
					// Aceptar una nueva conexión
					int new_client_fd = server->accept_connection();
					if (new_client_fd > 0) {
						add_client_to_poll(new_client_fd);
						std::cout << "New connection accepted on port "
						          << server->get_socket_fd() << std::endl;
					}
				} else {
					// Manejar la solicitud del cliente
					HttpRequestHandler request_handler;
					request_handler.handle_request(poll_fds[i].fd, server->get_config());

					// Cerrar la conexión del cliente
					close(poll_fds[i].fd);

					// Eliminar el descriptor de cliente de poll_fds
					poll_fds[i] = poll_fds[poll_fds.size() - 1];
					poll_fds.pop_back();
					--i;  // Ajustar el índice para verificar el nuevo descriptor en esta posición
				}
			}
		}
	}
}


