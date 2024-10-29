/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/16 21:19:10 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

void print_raw_lines(std::vector<std::string> rawLines) {
    for (std::vector<std::string>::iterator it = rawLines.begin(); it != rawLines.end(); it++) {
        std::cout << GRAY << *it << RESET << std::endl;
    }
}

void print_server_config(ServerConfig server)
{
    std::cout << GREEN << "Server parsed" << RESET << std::endl;
    std::cout << YELLOW << "  Port: " RESET << server.port << std::endl;
    std::cout << YELLOW << "  Server name: " RESET << server.server_name << std::endl;
    std::cout << YELLOW << "  Error pages: " RESET << server.error_pages.size() << std::endl;
    if (server.error_pages.size() > 0)
    {
        for (std::map<int, std::string>::iterator it = server.error_pages.begin(); it != server.error_pages.end(); it++)
        {
            std::cout << YELLOW << "    Error page: " RESET << it->first << " -> " << it->second << std::endl;
        }
    }
    std::cout << YELLOW << "  Client max body size: " RESET << server.client_max_body_size << std::endl;
    std::cout << YELLOW << "  Autoindex: " RESET << server.autoindex << std::endl;
    std::cout << YELLOW << "  Server root: " RESET << server.server_root << std::endl;
    std::cout << YELLOW << "  Default pages: " RESET << server.default_pages.size() << std::endl;
    if (server.default_pages.size() > 0)
    {
        for (std::vector<std::string>::iterator it = server.default_pages.begin(); it != server.default_pages.end(); it++)
        {
            std::cout << YELLOW << "    Default page: " RESET << *it << std::endl;
        }
    }

    std::cout << YELLOW << "  Webserver root: " RESET << server.ws_root << std::endl;
}
