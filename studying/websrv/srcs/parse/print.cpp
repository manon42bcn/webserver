/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/26 00:26:23 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

// This file and the functions can be deleted in the future

/**
 * @brief Prints raw lines from a vector of strings.
 *
 * @param rawLines The vector of strings to print.
 */
void print_raw_lines(std::vector<std::string> rawLines) {
    for (std::vector<std::string>::iterator it = rawLines.begin(); it != rawLines.end(); it++) {
        std::cout << GRAY << *it << RESET << std::endl;
    }
}

/**
 * @brief Prints the server configuration.
 *
 * @param server The server configuration to print.
 */
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
    if (server.locations.size() > 0)
    {
        std::cout << YELLOW << "  Locations: " RESET << server.locations.size() << std::endl;
        for (std::map<std::string, LocationConfig>::iterator it = server.locations.begin(); it != server.locations.end(); it++)
        {
            print_location_config(it->second);
        }
    }
}

/**
 * @brief Prints the location configuration.
 *
 * @param location The location configuration to print.
 */
void print_location_config(LocationConfig location) {
    std::cout << GRAY << "          Location root: " << location.loc_root << RESET << std::endl;
    // std::cout << GRAY << "          Location access: " << location.loc_access << RESET << std::endl;
    // std::cout << GRAY << "          Location default pages: " << location.loc_default_pages << RESET << std::endl;
    for (std::vector<std::string>::iterator it = location.loc_default_pages.begin(); it != location.loc_default_pages.end(); it++) {
        std::cout << GRAY << "          Location default page: " << *it << RESET << std::endl;
    }
    // std::cout << GRAY << "          Location error pages: " << location.loc_error_pages << RESET << std::endl;
    for (std::map<int, std::string>::iterator it = location.loc_error_pages.begin(); it != location.loc_error_pages.end(); it++) {
        std::cout << GRAY << "          Location error page: " << it->first << " -> " << it->second << RESET << std::endl;
    }
    // std::cout << GRAY << "          Location error mode: " << location.loc_error_mode << RESET << std::endl;
    // std::cout << GRAY << "          Location allowed methods: " << location.loc_allowed_methods << RESET << std::endl;
    // if (!location.loc_allowed_methods.empty())
    // {
    //     std::cout << GRAY << "          Location allowed methods: " << RESET;
    //     for (std::vector<t_methods>::iterator it = location.loc_allowed_methods.begin(); it != location.loc_allowed_methods.end(); it++) {
    //     if (*it == GET)
    //         std::cout << GRAY<< "GET " << RESET;
    //     else if (*it == POST)
    //         std::cout << GRAY << "POST " << RESET;
    //     else if (*it == DELETE)
    //         std::cout << GRAY << "DELETE " << RESET;
    //     else if (*it == PUT)
    //         std::cout << GRAY << "PUT " << RESET;
    //     else if (*it == HEAD)
    //         std::cout << GRAY << "HEAD " << RESET;
    //     else if (*it == OPTIONS)
    //         std::cout << GRAY << "OPTIONS " << RESET;
    //     else if (*it == TRACE)
    //         std::cout << GRAY << "TRACE " << RESET;
    //     else if (*it == CONNECT)
    //         std ::cout << GRAY << "CONNECT " << RESET;
    //     }
    //     std::cout << std::endl;
    // }
}

void print_servers(std::vector<ServerConfig> servers)
{
    for (std::vector<ServerConfig>::iterator it = servers.begin(); it != servers.end(); it++)
    {
        print_server_config(*it);
    }
}