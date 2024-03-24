/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserver.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/03/24 18:47:49 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"
#include "Config.hpp"

#include <iostream>
#include <cstring> // Para strerror y memset
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> // Para struct addrinfo, getnameinfo
#include <unistd.h> // Para close

bool checkArgs(int argc, char **argv)
{
    std::ifstream file;
    file.open(argv[1]);

    if (argc < 2)
    {
        std::cout << "Args error. Type --help for instructions." << std::endl;
        return (false);
    }
    if (!file)
    {
        std::cout << "Error: file \"" << argv[1] << "\" cant be opened." << std::endl;
        return (false);
    }
    //Add other errors in file, access...

    return (true);
}

void send_answer(int socket_client) {
    const char *responseHTTP =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n\r\n"
        "<!DOCTYPE html>\r\n"
        "<html>\r\n"
        "<head><title>Test Page</title></head>\r\n"
        "<body>\r\n"
        "<h1>Hello, World!</h1>\r\n"
        "</body>\r\n"
        "</html>\r\n";

    send(socket_client, responseHTTP, ft_strlen(responseHTTP), 0);
}

int main(int argc, char *argv[]) {
    bool debug = true;
    if (!checkArgs(argc, argv))
        exit(1);
    Config config(argv[1]);

    // std::cout << config.getValue("error_page") << std::endl;
    
    // Steps
    // int socket(int domain, int type, int protocol);
    // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    // int listen(int sockfd, int backlog);
    // int accept(int sockfd, struct sockaddr *_Nullable restrict addr, socklen_t *_Nullable restrict addrlen);
    // ssize_t recv(int sockfd, void buf[.len], size_t len, int flags);
    // ssize_t send(int sockfd, const void buf[.len], size_t len, int flags);
    
    struct addrinfo conf;
    ft_memset(&conf, 0, sizeof(struct addrinfo));
    // conf.ai_flags = AI_PASSIVE;
    conf.ai_family = AF_UNSPEC; // Ipv4(AF_INET) y Ipv6(AF_INET6)
    conf.ai_socktype = SOCK_STREAM; // TCP 

    struct addrinfo *result;


    if (getaddrinfo("127.0.0.1", "8080", &conf, &result) != 0) {
        std::cout << "Error getaddrinfo: " << gai_strerror(errno) << std::endl;
        return -1;
    }

    int unsocket = -1;
    struct addrinfo *ptr;
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        if (debug)
            std::cout << "For values: ai_family: " << ptr->ai_family << ", ai_socktype: " << ptr->ai_socktype << ", ai_protocol: " << ptr->ai_protocol << std::endl;
        unsocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (unsocket != -1) {
            if (debug)
                std::cout << "Socket created successful!. Decriptor: " << unsocket << std::endl;
            if (bind(unsocket, ptr->ai_addr, ptr->ai_addrlen) == 0) {
                if (debug)
                    std::cout << "Bind created successful!: " << unsocket << std::endl;
                break;
            }
            else if (debug)
                std::cout << "Error in bind for the socket: " << unsocket << std::endl;
            close(unsocket);
            if (debug)
                std::cout << "Socket closed: " << unsocket << std::endl;
            unsocket = -1;
        }
        else if (debug)
            std::cout << "Error creating the socket" << std::endl;
    }

    freeaddrinfo(result);

    if (unsocket == -1) {
        std::cout << "Error opening the socket" << std::endl;
        return -1;
    }

    if (listen(unsocket, 10) != 0) { // Maximum of connections 10?
        std::cout << "Error listen: " << strerror(errno) << std::endl;
        close(unsocket);
        return -1;
    }

    struct sockaddr_storage dircliente;
    socklen_t longdircliente;

    while (true) {
        longdircliente = sizeof(dircliente);

        int socketcliente = accept(unsocket, (struct sockaddr *)&dircliente, &longdircliente);
        if (socketcliente != -1) {
            send_answer(socketcliente);
            close(socketcliente);
        }
    }
    close(unsocket);
    return 0;
}
