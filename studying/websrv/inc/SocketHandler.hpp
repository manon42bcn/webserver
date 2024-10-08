#ifndef SOCKETHANDLER_HPP
#define SOCKETHANDLER_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>

# define EXIT_FAILURE 1
# define EXIT_SUCCESS 0
# define ERR_CNN -1

# define SOCKET_CREATE "Create Socket."
# define SOCKET_CONFIG "Config Socket."
# define SOCKET_BIND "Bind Socket to Port: "
# define SOCKET_LISTEN "Listen at Port: "
# define SOCKET_FLAGS "Socket flags."
# define SOCKET_NON_BLOCK "Configure Socket as Non Blocking."
# define SERVER_LISTEN "Server listening Port: "
# define CONN_ACCEPTED "Accepting Connection"

// TODO: Clean and made it orthodox
/**
 * @brief Class SocketHandler
 *
 * This class encapsulate the sockets, creation, configuration
 * and connexions
 *
 */
class SocketHandler {
private:
	int port;
	int socket_fd;
	struct sockaddr_in address;

public:
	SocketHandler(int port);
	void configure();
	int accept_connection();
	int get_socket_fd() const;
	void close_socket();

private:
	void set_nonblocking(int fd);
};

#endif
