/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/26 00:27:00 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"
#include <set>

LocationConfig parse_location_block(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end) {
    std::map<std::string, std::string> locations;
    LocationConfig location;

    if (find_exact_string(*start, "location"))
        location.loc_root = std::string("");
        // location.loc_root = delete_brackets_clean(get_value(*start, "location"));

    // std::cout << "Location block found" << std::endl;
    for (std::vector<std::string>::iterator it = start; it != end; it++) {
        // std::cout << "Location block line: " <<RED << *it << RESET << std::endl;
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
            if (check_error_page(get_value(*it, "error_page")))
                location.loc_error_pages = split_error_pages(get_value(*it, "error_page"));
            else
            {
                std::cout << "Error: error_page " << get_value(*it, "error_page") << " is not valid." << std::endl;
                exit(1);
            }
        }
        else if (find_exact_string(*it, "limit_except"))
        {
            std::cout << "Limit except found" << std::endl;
            // std::vector<t_methods> methods = split_allowed_methods(get_value(*it, "limit_except"));

            // Definimos todos los métodos posibles
            // t_methods allMethodsArray[] = {GET, POST, DELETE, PUT, HEAD, OPTIONS, TRACE, CONNECT};
            // std::vector<t_methods> allMethods(allMethodsArray, allMethodsArray + sizeof(allMethodsArray) / sizeof(t_methods));

            // for (std::vector<t_methods>::iterator it = allMethods.begin(); it != allMethods.end(); ++it) {
            //     if (std::find(methods.begin(), methods.end(), *it) == methods.end()) {
            //         location.loc_allowed_methods.push_back(*it);
            //     }
            // }
        }

        if (it->find("}") != std::string::npos)
            break;
    }
    // std::cout << "Location block end" << std::endl;
    location.loc_access = ACCESS_WRITE;
    // for (std::map<std::string, std::string>::iterator it = locations.begin(); it != locations.end(); it++) {
    //     std::cout << "Location block: " << it->first << " - " << it->second << std::endl;
    // }
    // std::cout << location << std::endl;
    // return 

    // print_location_config(location);
    return location;
    // exit(0);
}















ServerConfig parse_server_block(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end, Logger* logger)
{
    ServerConfig server;
    
    start++;
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing server block");

    for (std::vector<std::string>::iterator it = start; it != end; ++it)
    {
        if (find_exact_string(*it, "location"))
        {
            LocationConfig location = parse_location_block(it, end);
            // std::vector<std::string>::iterator start_location = it;
            // std::vector<std::string>::iterator end_location = find_block_end(start_location, end);
            location = parse_location_block(it, find_block_end(it, end));
            server.locations["/"] = location;
            it = skip_block(it, find_block_end(it, end));
            // it = end_location;
        }
        else if (find_exact_string(*it, "port"))
        {
            int port = check_port(get_value(*it, "port"));
            if (port != -1)
                server.port = port;
            else
                logger->fatal_log("parse_server_block", "Port " + get_value(*it, "port") + " is not valid.");
        }
        else if (find_exact_string(*it, "server_name"))
        {
            std::string serverName = get_value(*it, "server_name");
            logger->log(LOG_DEBUG, "parse_server_block", "Server name: " + serverName);
            if (check_server_name(serverName))
                server.server_name = serverName;
            else
                logger->fatal_log("parse_server_block", "Server name " + serverName + " is not valid.");
        }
        else if (find_exact_string(*it, "root"))
        {
            std::string root = get_value(*it, "root");
            if (check_root(root))
                server.server_root = get_server_root() + get_value(*it, "root");
            else
                logger->fatal_log("parse_server_block", "Server root " + root + " is not valid.");
        }
        else if (find_exact_string(*it, "index"))
        {
            if (check_default_page(get_value(*it, "index")))
                server.default_pages = split_default_pages(get_value(*it, "index"));
            else
                logger->fatal_log("parse_server_block", "Default page " + get_value(*it, "index") + " is not valid.");
        }
        else if (find_exact_string(*it, "client_max_body_size"))
        {
            std::string client_max_body_size = get_value(*it, "client_max_body_size");
            if (check_client_max_body_size(client_max_body_size))
                server.client_max_body_size = get_value(*it, "client_max_body_size");
            else
                logger->fatal_log("parse_server_block", "Client max body size " + client_max_body_size + " is not valid.");
        }
        else if (find_exact_string(*it, "error_page"))
        {
            std::string error_page = get_value(*it, "error_page");
            if (check_error_page(error_page))
            {
                std::map<int, std::string> new_error_pages = split_error_pages(error_page);
                server.error_pages.insert(new_error_pages.begin(), new_error_pages.end());
            }
            else
                logger->fatal_log("parse_server_block", "Error page " + error_page + " is not valid.");
        }
        else if (find_exact_string(*it, "autoindex"))
        {
            if (check_autoindex(get_value(*it, "autoindex")))
                server.autoindex = true;
            else if (check_autoindex(get_value(*it, "autoindex")))
                server.autoindex = false;
            else
                logger->fatal_log("parse_server_block", "Autoindex " + get_value(*it, "autoindex") + " is not valid.");
        }
        else
        {
            std::cout << RED << "Error: " << *it << " is not valid parameter" << RESET << std::endl;
            continue;
        }
        // else
        //     it++;
        // else if (it->empty() || find_exact_string(*it, "allowedMethods") || find_exact_string(*it, "}") || find_exact_string(*it, "{"))
        //     continue;
        // else
        // {
        //     std::cout << RED << "Error: " << *it << " is not valid parameter" << RESET << std::endl;
        //     exit(1);
        // }
    }

    if (server.server_name == "")
        server.server_name = "localhost";
    if (!server.autoindex)
        server.autoindex = false;
    server.ws_root = get_server_root();

    // logger->log(LOG_DEBUG, "parse_server_block", "Server NAME: " + server.server_name + " PORT: " + int_to_string(server.port));

    // if (!server.checkObligatoryParams())
    //     throw std::runtime_error("Obligatory parameters are missing in server block in server " + server.getServerName());
    //  verificar si tengo 2 veces el mismo parametro
    return server;
}

std::vector<ServerConfig> parse_servers(std::vector<std::string> rawLines, Logger* logger)
{
    std::vector<std::string>::iterator start;
    std::vector<std::string>::iterator end;
    std::vector<ServerConfig> servers;

    logger->log(LOG_DEBUG, "parse_servers", "Checking brackets");
    if (!check_brackets(rawLines.begin(), rawLines.end()))  
        logger->log(LOG_ERROR, "parse_servers", "Brackets are not closed");
    for (std::vector<std::string>::iterator it = rawLines.begin(); it != rawLines.end(); it++)
    {
        if (find_exact_string(*it, "server"))
        {
            logger->log(LOG_DEBUG, "parse_servers", "Server block found");
            start = it;
            end =   find_block_end(start, rawLines.end());
            servers.push_back(parse_server_block(start, end, logger));
            it = skip_block(start, end);
        }
    }

    logger->log(LOG_INFO, "parse_servers", "Servers found: " + int_to_string(servers.size()));
    print_servers(servers);
    return servers;
}

std::vector<ServerConfig> parse_file(std::string path, Logger* logger)
{
    logger->log(LOG_DEBUG, "parse_file", "Parsing file: " + path);
    std::vector<std::string> rawLines = get_raw_lines(path);
    logger->log(LOG_DEBUG, "parse_file", "Raw lines obtained: " + int_to_string(rawLines.size()));
    std::vector<ServerConfig> servers = parse_servers(rawLines, logger);
    return servers;
}
