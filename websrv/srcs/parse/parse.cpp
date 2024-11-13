/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/11/13 23:26:06 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

LocationConfig parse_location_block(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end, Logger* logger) {
    LocationConfig location;

    start++;
    for (std::vector<std::string>::iterator it = start; it != end; it++) {
        if (find_exact_string(*it, "index"))
            parse_location_index(it, logger, location);
        else if (find_exact_string(*it, "error_page"))
            parse_location_error_page(it, logger, location);
        else if (find_exact_string(*it, "root"))
            parse_root(it, logger, location);
        else if (find_exact_string(*it, "autoindex"))
            parse_autoindex(it, logger, location);
        else if (find_exact_string(*it, "cgi"))
            parse_cgi(it, logger, location);
        else if (find_exact_string(*it, "error_mode"))
            parse_template_error_page(it, logger, location);
        else if (find_exact_string(*it, "accept_only"))
            parse_accept_only(it, logger, location);
        else if (it->find("}") != std::string::npos)
        {
            // Realmente no se si llega aqui
            logger->log(LOG_DEBUG, "parse_location_block", "Saliendo porque encontre } en location block");
            break;
        }
        else
            logger->fatal_log("parse_location_block", "Error: [" + *it + "] is not valid parameter in location block");
    }



    // Obligatorios
    // if (location.loc_root == "")
    //    logger->fatal_log("parse_location_block", "Location root is not valid.");
    if (location.loc_error_pages.size() != 0)
    {
        for (std::map<int, std::string>::iterator it = location.loc_error_pages.begin(); it != location.loc_error_pages.end(); it++)
        {
            it->second = join_paths(get_server_root(), it->second);
        }
    }
    return location;
}

ServerConfig parse_server_block(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end, Logger* logger)
{
    ServerConfig server;
    
    start++;
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing server block");
    for (std::vector<std::string>::iterator it = start; it != end; ++it)
    {
        if (find_exact_string(*it, "location"))
            parse_location(it, end, logger, server);
        else if (find_exact_string(*it, "template_error_page"))
            parse_template_error_page(it, logger, server);
        else if (find_exact_string(*it, "port"))
            parse_port(it, logger, server);
        else if (find_exact_string(*it, "server_name"))
            parse_server_name(it, logger, server);
        else if (find_exact_string(*it, "root"))
            parse_root(it, logger, server);
        else if (find_exact_string(*it, "index"))
            parse_index(it, logger, server);
        else if (find_exact_string(*it, "client_max_body_size"))
            parse_client_max_body_size(it, logger, server);
        else if (find_exact_string(*it, "error_page"))
            parse_error_page(it, logger, server);
        else if (find_exact_string(*it, "autoindex"))
            parse_autoindex(it, logger, server);
        else if (find_exact_string(*it, "error_mode"))
            parse_error_mode(it, logger, server);
        else if (it->find("}") != std::string::npos)
        {
            // deberia ser break o continue?, no se si llega aqui
            logger->log(LOG_DEBUG, "parse_server_block", "Saliendo porque encontre } en server block");
            break;
        }
        else
            logger->fatal_log("parse_server_block", "Error: [" + *it + "] is not valid parameter in server block");
    }
    logger->log(LOG_DEBUG, "parse_server_block", "Server block parsed");

    // Configuracion de variables
    server.ws_root = server.server_root;
    // server.ws_root = "/Users/vaguilar/Desktop/webserver/websrv//data";
    if (server.error_pages.size() > 0 && server.server_root != "")
    {
        logger->log(LOG_DEBUG, "parse_server_block", "Joining error pages");
        for (std::map<int, std::string>::iterator it = server.error_pages.begin(); it != server.error_pages.end(); it++)
            it->second = join_paths(server.server_root, it->second);
    }

    if (check_obligatory_params(server, logger))
        logger->fatal_log("parse_server_block", "Obligatory parameters are not valid.");
    
    return server;
}

std::vector<ServerConfig> parse_servers(std::vector<std::string> rawLines, Logger* logger)
{
    std::vector<std::string>::iterator start;
    std::vector<std::string>::iterator end;
    std::vector<ServerConfig> servers;

    logger->log(LOG_DEBUG, "parse_servers", "Checking brackets");
    if (!check_brackets(rawLines.begin(), rawLines.end()))  
        logger->fatal_log("parse_servers", "Brackets are not closed");
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
    logger->log(LOG_DEBUG, "parse_servers", "Locations found: " + int_to_string(servers.size()));
    print_servers(servers);
    if (check_duplicate_servers(servers))
        logger->fatal_log("parse_servers", "Duplicate servers found");
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
