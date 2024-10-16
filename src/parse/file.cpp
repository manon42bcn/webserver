/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   file.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/16 23:16:45 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"
#include <set>

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
    if (!location.loc_allowed_methods.empty())
    {
        std::cout << GRAY << "          Location allowed methods: " << RESET;
        for (std::vector<t_methods>::iterator it = location.loc_allowed_methods.begin(); it != location.loc_allowed_methods.end(); it++) {
        if (*it == GET)
            std::cout << GRAY<< "GET " << RESET;
        else if (*it == POST)
            std::cout << GRAY << "POST " << RESET;
        else if (*it == DELETE)
            std::cout << GRAY << "DELETE " << RESET;
        else if (*it == PUT)
            std::cout << GRAY << "PUT " << RESET;
        else if (*it == HEAD)
            std::cout << GRAY << "HEAD " << RESET;
        else if (*it == OPTIONS)
            std::cout << GRAY << "OPTIONS " << RESET;
        else if (*it == TRACE)
            std::cout << GRAY << "TRACE " << RESET;
        else if (*it == CONNECT)
            std ::cout << GRAY << "CONNECT " << RESET;
        }
        std::cout << std::endl;
    }
}

std::vector<t_methods> split_allowed_methods(std::string allowed_methods)
{
    std::vector<t_methods> methods;
    std::istringstream iss(allowed_methods);
    std::string method;
    while (iss >> method) {
        if (method == "GET")
            methods.push_back(GET);
        else if (method == "POST")
            methods.push_back(POST);
        else if (method == "DELETE")
            methods.push_back(DELETE);
        else if (method == "PUT")
            methods.push_back(PUT);
        else if (method == "HEAD")
            methods.push_back(HEAD);
    }
    return methods;
}



void handleLocationBlock(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end) {
    std::map<std::string, std::string> locations;
    LocationConfig location;

    if (find_exact_string(*start, "location"))
        location.loc_root = delete_brackets_clean(get_value(*start, "location"));

    std::cout << "Location block found" << std::endl;
    for (std::vector<std::string>::iterator it = start; it != end; it++) {
        std::cout << "Location block line: " <<RED << *it << RESET << std::endl;
        if (find_exact_string(*it, "index"))
        {
            if (!check_default_page(get_value(*it, "index")))
            {
                std::cout << "Error: default page " << get_value(*it, "index") << " is not valid." << std::endl;
                exit(1);
            }
            location.loc_default_pages = split_default_pages(get_value(*it, "index"));
        }
        else if (find_exact_string(*it, "error_page"))
        {
            // std::cout << "Error page found" << std::endl;
            if (check_error_page(get_value(*it, "error_page")))
            {
                location.loc_error_pages = split_error_pages(get_value(*it, "error_page"));
            }
            else
            {
                std::cout << "Error: error_page " << get_value(*it, "error_page") << " is not valid." << std::endl;
                exit(1);
            }
        }
        else if (find_exact_string(*it, "limit_except"))
        {
            std::cout << "Limit except found" << std::endl;
            std::vector<t_methods> methods = split_allowed_methods(get_value(*it, "limit_except"));

            // Definimos todos los métodos posibles
            t_methods allMethodsArray[] = {GET, POST, DELETE, PUT, HEAD, OPTIONS, TRACE, CONNECT};
            std::vector<t_methods> allMethods(allMethodsArray, allMethodsArray + sizeof(allMethodsArray) / sizeof(t_methods));

            // Iteramos sobre todos los métodos y añadimos los que no están en el vector de métodos permitidos
            for (std::vector<t_methods>::iterator it = allMethods.begin(); it != allMethods.end(); ++it) {
                if (std::find(methods.begin(), methods.end(), *it) == methods.end()) {
                    location.loc_allowed_methods.push_back(*it);
                }
            }
        }

        if (it->find("}") != std::string::npos)
            break;
    }
    std::cout << "Location block end" << std::endl;
    // for (std::map<std::string, std::string>::iterator it = locations.begin(); it != locations.end(); it++) {
    //     std::cout << "Location block: " << it->first << " - " << it->second << std::endl;
    // }
    // std::cout << location << std::endl;
    // return 

    print_location_config(location);
    exit(0);
}







std::vector<std::string>::iterator findServerBlockEnd(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end)
{
    int bracketCount = 0;

    for (std::vector<std::string>::iterator it = start; it != end; ++it)
    {
        if (it->find("{") != std::string::npos)
        {
            bracketCount++;
        }
        if (it->find("}") != std::string::npos)
        {
            bracketCount--;
            if (bracketCount == 0)
            {
                return it;
            }
        }
    }
    return end;
}



std::vector<std::string>::iterator findLocationBlockEnd(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end) {
    int bracketCount = 0;
    for (std::vector<std::string>::iterator it = start; it != end; ++it) {
        if (it->find("{") != std::string::npos) {
            bracketCount++;
        }
        if (it->find("}") != std::string::npos) {
            bracketCount--;
            if (bracketCount == 0) {
                return it;
            }
        }
    }
    return end;
}







std::string get_server_root()
{
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) != NULL) {
        return std::string(buffer);
    }
    return "";
}

ServerConfig parseServerBlock(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end)
{
    ServerConfig server;
    std::string temp;
    int temp_int;
    
    start++;

    
    for (std::vector<std::string>::iterator it = start; it != end; ++it)
    {
        if (find_exact_string(*it, "location"))
        {
            std::vector<std::string>::iterator start_location = it;
            std::vector<std::string>::iterator end_location = findLocationBlockEnd(start_location, end);
            handleLocationBlock(start_location, end_location);
            // it = skip_block(start_location, end_location);
            // it = end_location;
        }
        else if (find_exact_string(*it, "port"))
        {
            temp_int = check_port(get_value(*it, "port"));
            if (temp_int > 0)
                server.port = temp_int;
            else
            {
                std::cout << "Error: port " << temp << " is not valid." << std::endl;
                exit(1);
            }
        }
        else if (find_exact_string(*it, "server_name"))
        {
            if (!check_server_name(get_value(*it, "server_name")))
            {
                std::cout << "Error: server_name " << get_value(*it, "server_name") << " is not valid." << std::endl;
                exit(1);
            }
            server.server_name = get_value(*it, "server_name");
        }
        else if (find_exact_string(*it, "error_page"))
        {

            if (check_error_page(get_value(*it, "error_page")))
            {
                server.error_pages = split_error_pages(get_value(*it, "error_page"));
            }
            else
            {
                std::cout << "Error: error_page " << get_value(*it, "error_page") << " is not valid." << std::endl;
                exit(1);
            }
        }
        else if (find_exact_string(*it, "client_max_body_size"))
            server.client_max_body_size = get_value(*it, "client_max_body_size");
        else if (find_exact_string(*it, "autoindex"))
        {
            if (get_value(*it, "autoindex") == "on")
                server.autoindex = true;
            else if (get_value(*it, "autoindex") == "off")
                server.autoindex = false;
            else
            {
                std::cout << "Error: autoindex " << get_value(*it, "autoindex") << " is not valid." << std::endl;
                exit(1);
            }
        }
        else if (find_exact_string(*it, "root"))
            server.server_root = get_value(*it, "root");
        else if (find_exact_string(*it, "index"))
        {
            if (!check_default_page(get_value(*it, "index")))
            {
                std::cout << "Error: default page " << get_value(*it, "index") << " is not valid." << std::endl;
                exit(1);
            }
            server.default_pages = split_default_pages(get_value(*it, "index"));
        }
        else
            continue;
        // else
        //     it++;
        // else if (it->empty() || find_exact_string(*it, "allowedMethods") || find_exact_string(*it, "}") || find_exact_string(*it, "{"))
        //     continue;
        // else
        // {
        //     std::cout << RED << "Error: " << *it << " is not valid parameter" << RESET << std::endl;
        //     exit(1);
        // }
        // std::cout << GRAY << "Server block line: " << *it << RESET << std::endl;
    }
    // std::cout << "Server block end out of for" << std::endl;

    if (server.server_name == "")
        server.server_name = "localhost";
    server.ws_root = get_server_root();

    // if (!server.checkObligatoryParams())
    //     throw std::runtime_error("Obligatory parameters are missing in server block in server " + server.getServerName());
    //  verificar si tengo 2 veces el mismo parametro


    print_server_config(server);
    return server;
}

std::vector<std::string>::iterator skip_block(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end)
{
    int bracketCount = 0;
    for (std::vector<std::string>::iterator it = start; it != end; ++it)
    {
        if (it->find("{") != std::string::npos)
        {
            bracketCount++;
        }
        if (it->find("}") != std::string::npos)
        {
            bracketCount--;
            if (bracketCount == 0)
            {
                return ++it;
            }
        }
    }
    return end;
}


std::vector<ServerConfig> parse_servers(std::vector<std::string> rawLines)
{
    std::vector<std::string>::iterator start;
    std::vector<std::string>::iterator end;
    std::vector<ServerConfig> servers;
    if (!check_brackets(rawLines.begin(), rawLines.end()))
    {
        std::cout << "Error: Brackets are not closed" << std::endl;
        exit(1);
    }
    for (std::vector<std::string>::iterator it = rawLines.begin(); it != rawLines.end(); it++)
    {
        if (find_exact_string(*it, "server"))
        {
            std::cout << "Server block found" << std::endl;
            start = it;
            end = findServerBlockEnd(start, rawLines.end());
            servers.push_back(parseServerBlock(start, end));
            it = skip_block(start, end);
        }
    }

    std::cout << "Servers found: " << servers.size() << std::endl;
    return servers;
}

void parse_file(std::string path)
{
    std::vector<std::string> rawLines = get_raw_lines(path);
    std::vector<ServerConfig> servers = parse_servers(rawLines);
}



