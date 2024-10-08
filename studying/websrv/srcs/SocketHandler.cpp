#include "SocketHandler.hpp"

/**
 * @brief SocketHandler Constructor
 *
 * Set port and fd for socket. -1 means that the fd has been not set yet.
 *
 * @param port Port that the socket will hear.
 */
SocketHandler::SocketHandler(int port) : port(port), socket_fd(-1) {}

/**
 * @brief Configures the socket for listening on the specified port.
 *
 * This function creates a socket, sets it to non-blocking mode,
 * and binds it to the specified port.
 *
 * @param port The port number to listen on.
 */
void SocketHandler::configure() {
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_fd == 0) {
		std::cerr << "Error: " << SOCKET_CREATE << std::endl;
		exit(EXIT_FAILURE);
	}

	int opt = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		std::cerr << "Error: " << SOCKET_CONFIG << std::endl;
		close(socket_fd);
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		std::cerr << "Error: " << SOCKET_BIND << port << std::endl;
		close(socket_fd);
		exit(EXIT_FAILURE);
	}

	if (listen(socket_fd, 3) < 0) {
		std::cerr << "Error: " << SOCKET_LISTEN << port << std::endl;
		close(socket_fd);
		exit(EXIT_FAILURE);
	}
	set_nonblocking(socket_fd);

	std::cout << SERVER_LISTEN << port << std::endl;
}
/**
 * @brief Accepts an incoming connection on the listening socket.
 *
 * This function accepts a new connection from a client and returns
 * the file descriptor of the new socket.
 *
 * @return The file descriptor of the new socket, or -1 if an error occurred.
 */
int SocketHandler::accept_connection() {
	int new_socket;
	socklen_t addrlen = sizeof(address);
	new_socket = accept(socket_fd, (struct sockaddr *)&address, &addrlen);
	if (new_socket < 0) {
		std::cerr << "Error :" << CONN_ACCEPTED << std::endl;
		return (-1);
	}

	set_nonblocking(new_socket);
	return (new_socket);
}

/**
 * @brief Gets the file descriptor of the listening socket.
 *
 * @return The file descriptor of the listening socket.
 */
int SocketHandler::get_socket_fd() const {
	return (socket_fd);
}

/**
 * @brief Closes the listening socket.
 */
void SocketHandler::close_socket() {
	close(socket_fd);
}

/**
 * @brief Sets the specified file descriptor to non-blocking mode.
 *
 * Non blocking mode, means that the workflow will not be affected if
 * A read/write process is not finished.
 *
 * @param fd The file descriptor to set to non-blocking mode.
 */
void SocketHandler::set_nonblocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		std::cerr << "Error: " << SOCKET_FLAGS << std::endl;
		exit(EXIT_FAILURE);
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		std::cerr << "Error: " << SOCKET_NON_BLOCK << std::endl;
		exit(EXIT_FAILURE);
	}
}