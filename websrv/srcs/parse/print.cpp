/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/11/17 01:43:51 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

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
    std::cout << YELLOW << "  Autoindex: " RESET << (static_cast<bool>(server.autoindex) ? "true" : "false") << std::endl;
    std::cout << YELLOW << "  Server root: " RESET << server.server_root << std::endl;
    std::cout << YELLOW << "  Default pages: " RESET << server.default_pages.size() << std::endl;
    if (server.default_pages.size() > 0)
    {
        for (std::vector<std::string>::iterator it = server.default_pages.begin(); it != server.default_pages.end(); it++)
        {
            std::cout << YELLOW << "    Default page: " RESET << *it << std::endl;
        }
    }

    std::cout << YELLOW << "  Template error page: " RESET << server.template_error_page << std::endl;
    std::cout << YELLOW << "  Error mode: " RESET << (server.error_mode == LITERAL ? "literal" : "template") << std::endl;

    std::cout << YELLOW << "  Webserver root: " RESET << server.ws_root << std::endl;
    if (server.locations.size() > 0)
    {
        std::cout << YELLOW << "  Locations: " RESET << server.locations.size() << std::endl;
        for (std::map<std::string, LocationConfig>::iterator it = server.locations.begin(); it != server.locations.end(); it++)
        {
            std::cout << YELLOW << "    Location path: " RESET << it->first << std::endl;
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
    std::cout << GRAY << "      Location root: " << location.loc_root << RESET << std::endl;
    std::cout << GRAY << "      Default pages: " RESET << location.loc_default_pages.size() << std::endl;
    if (location.loc_default_pages.size() > 0)
    {
        for (std::vector<std::string>::iterator it = location.loc_default_pages.begin(); it != location.loc_default_pages.end(); it++)
        {
            std::cout << GRAY << "        Default page: " RESET << *it << std::endl;
        }
    }
    std::cout << GRAY << "      Error pages: " RESET << location.loc_error_pages.size() << std::endl;
    if (location.loc_error_pages.size() > 0)
    {
        for (std::map<int, std::string>::iterator it = location.loc_error_pages.begin(); it != location.loc_error_pages.end(); it++)
        {
            std::cout << GRAY << "        Error page: " RESET << it->first << " -> " << it->second << std::endl;
        }
    }
    
    std::cout << GRAY << "      Autoindex: " RESET << (static_cast<bool>(location.autoindex) ? "true" : "false") << std::endl;

        
    std::cout << GRAY << "      Allowed methods: " RESET;
    int method_count = 0;
    for (int i = 0; i < 8; i++) {
        if (location.loc_allowed_methods & (1 << i)) {
            method_count++;
        }
    }
    std::cout << method_count << std::endl;
    
    for (int i = 0; i < 8; i++) {
        if (location.loc_allowed_methods & (1 << i)) {
            std::cout << GRAY << "        Allowed method: " RESET << print_bitwise_method(1 << i) << std::endl;
        }
    }
    std::cout << GRAY << "      CGI: " RESET << (static_cast<bool>(location.cgi_file) ? "true" : "false") << std::endl;
    std::cout << GRAY << "      Redirections: " RESET << location.redirections.size() << std::endl;
    if (location.redirections.size() > 0)
    {
        for (std::map<int, std::string>::iterator it = location.redirections.begin(); it != location.redirections.end(); it++)
        {
            std::cout << GRAY << "        Redirection: " RESET << it->first << " -> " << it->second << std::endl;
        }
    }
    std::cout << GRAY << "      Is root: " RESET << (location.is_root ? "true" : "false") << std::endl;
}

std::string print_bitwise_method(unsigned char method) {
    std::string method_string = "";     
    if (method & MASK_METHOD_GET)
        method_string += "GET ";
    if (method & MASK_METHOD_POST)
        method_string += "POST ";
    if (method & MASK_METHOD_DELETE)
        method_string += "DELETE ";
    if (method & MASK_METHOD_PUT)
        method_string += "PUT ";
    if (method & MASK_METHOD_PATCH)
        method_string += "PATCH ";
    if (method & MASK_METHOD_TRACE)
        method_string += "TRACE ";
    if (method & MASK_METHOD_OPTIONS)
        method_string += "OPTIONS ";
    if (method & MASK_METHOD_HEAD)
        method_string += "HEAD ";
    return method_string;
}

void print_servers(std::vector<ServerConfig> servers)
{
    for (std::vector<ServerConfig>::iterator it = servers.begin(); it != servers.end(); it++)
    {
        print_server_config(*it);
    }
    //exit(0);
}
