#include "webserver.hpp"
#include "ServerManager.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>


/**
 * @brief Constructor of ServerManager Class.
 *
 * @param configs vector with ServerConfig structs that include server configuration
 */
ServerManager::ServerManager(const std::vector<ServerConfig>& configs) {
	poll_fds.reserve(100);

	for (size_t i = 0; i < configs.size(); ++i) {
		add_server(configs[i].port, configs[i]);
	}
}

/**
 * @brief Add a new server in a given port
 *
 * @param port Port to listen
 * @param config ServerConfig struct with Server configuration
 */
void ServerManager::add_server(int port, const ServerConfig& config) {
	SocketHandler* server = new SocketHandler(port, config);
	servers.push_back(server);
	add_server_to_poll(server->get_socket_fd());
}

/**
 * @brief Añade el descriptor del socket del servidor al vector poll_fds.
 *
 * @param server_fd Descriptor del socket del servidor.
 */
void ServerManager::add_server_to_poll(int server_fd) {
	struct pollfd pfd;
	pfd.fd = server_fd;
	pfd.events = POLLIN;  // Estamos interesados en las operaciones de lectura
	pfd.revents = 0;
	poll_fds.push_back(pfd);
}

/**
 * @brief Añade el descriptor del socket del cliente al vector poll_fds.
 *
 * @param client_fd Descriptor del socket del cliente.
 */
void ServerManager::add_client_to_poll(int client_fd) {
	struct pollfd pfd;
	pfd.fd = client_fd;
	pfd.events = POLLIN;  // También queremos leer desde los clientes
	pfd.revents = 0;
	poll_fds.push_back(pfd);
}

/**
 * @brief Loop of events to handle listen, get request and send responses
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
						std::cout << "Nueva conexión aceptada en el puerto "
						          << server->get_socket_fd() << std::endl;
					}
				} else {
					// Manejar la solicitud del cliente
					HttpRequestHandler request_handler;
					request_handler.handle_request(poll_fds[i].fd, server->get_config()); // Obtener config desde SocketHandler

					// Cerrar la conexión del cliente
					close(poll_fds[i].fd);

					// Eliminar el descriptor de cliente del vector poll_fds
					poll_fds[i] = poll_fds[poll_fds.size() - 1];
					poll_fds.pop_back();
					--i; // Ajustar el índice para verificar el nuevo descriptor en esta posición
				}
			}
		}
	}
}
