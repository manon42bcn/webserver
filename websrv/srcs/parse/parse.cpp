/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/11/17 14:34:19 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

LocationConfig parse_location_block(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end, Logger* logger) {
    LocationConfig location;

    static const CommandPair commands[] = {
        {"index", parse_location_index},
        {"error_page", parse_location_error_page},
        {"root", parse_root},
        {"autoindex", parse_autoindex},
        {"cgi", parse_cgi},
        {"error_mode", parse_template_error_page},
        {"accept_only", parse_accept_only},
        {"redirection", parse_redirection},
        {NULL, NULL}
    };

    ++start;
    for (std::vector<std::string>::iterator it = start; it != end; ++it) {
        std::string first_word = get_first_word(*it);
        
        if (first_word == "}") {
            logger->log(LOG_DEBUG, "parse_location_block", "Fin del bloque de location");
            break;
        }

        bool found = false;
        for (const CommandPair* cmd = commands; cmd->command != NULL; ++cmd) {
            if (first_word == cmd->command) {
                cmd->function(it, logger, location);
                found = true;
                break;
            }
        }

        if (!found) {
            logger->fatal_log("parse_location_block", 
                             "Error: [" + first_word + "] no es un parametro valido en el bloque location");
        }
    }

    if (location.loc_error_pages.size() != 0)
    {
        for (std::map<int, std::string>::iterator it = location.loc_error_pages.begin(); it != location.loc_error_pages.end(); it++)
        {
            it->second = join_paths(get_server_root(), it->second);
        }
    }
    if (location.loc_allowed_methods == 0)
        GRANT_ALL(location.loc_allowed_methods);
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
            logger->log(LOG_DEBUG, "parse_server_block", "Saliendo porque encontre } en server block");
            break;
        }
        else
            logger->fatal_log("parse_server_block", "Error: [" + *it + "] is not valid parameter in server block");
    }
    
    server.ws_root = get_server_root();
    if (server.error_pages.size() > 0 && server.server_root != "")
    {
        logger->log(LOG_DEBUG, "parse_server_block", "Joining error pages");
        for (std::map<int, std::string>::iterator it = server.error_pages.begin(); it != server.error_pages.end(); it++)
            it->second = join_paths(server.server_root, it->second);
    }
    for (std::map<std::string, LocationConfig>::iterator it = server.locations.begin(); it != server.locations.end(); it++) {
        if (it->second.loc_root != "")
            it->second.loc_root = join_paths(server.server_root, it->second.loc_root);
        if (compare_paths(it->first, it->second.loc_root))
            it->second.is_root = true;
    }

    if (check_obligatory_params(server, logger))
        logger->fatal_log("parse_server_block", "Obligatory parameters are not valid.");
    std::cout << YELLOW << "server.default_pages is " << server.default_pages[0] << RESET << std::endl;
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
    if (check_duplicate_servers(servers))
        logger->fatal_log("parse_servers", "Duplicate servers found");
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
