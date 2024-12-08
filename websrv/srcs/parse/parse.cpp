/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/12/04 23:49:32 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

/**
 * @brief Parses a location block configuration from the config file.
 *
 * Processes location-specific directives and builds a LocationConfig object.
 * Handles various location settings including index files, error pages,
 * root paths, and access controls.
 *
 * @param start Iterator to the start of the location block.
 * @param end Iterator to the end of the configuration file.
 * @param logger Pointer to the logger instance.
 * @return LocationConfig Parsed location configuration.
 * @throw Logger::fatal_log if invalid parameters are encountered.
 */
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
                             "Error: \"" + first_word + "\" is not a valid parameter in the location block");
        }
    }

    if (!location.path_root.empty() && location.path_root[location.path_root.size() - 1] == '/')
        location.path_root.erase(location.path_root.size() - 1);
    if (location.loc_allowed_methods == 0)
        GRANT_ALL(location.loc_allowed_methods);
    return location;
}

/**
 * @brief Parses a server block configuration from the config file.
 *
 * Processes server-level directives and builds a ServerConfig object.
 * Handles various server settings including ports, server names,
 * and location blocks.
 *
 * @param start Iterator to the start of the server block.
 * @param end Iterator to the end of the configuration file.
 * @param logger Pointer to the logger instance.
 * @return ServerConfig Parsed server configuration.
 * @throw Logger::fatal_log if mandatory parameters are missing or invalid.
 */
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
            logger->fatal_log("parse_server_block", "Found } in server block");
            break;
        }
        else
            logger->fatal_log("parse_server_block", "Error: \"" + *it + "\" is not valid parameter in server block");
    }
    
    server.ws_root = get_server_root();
    if (server.error_pages.size() > 0 && server.server_root != "")
    {
        logger->log(LOG_DEBUG, "parse_server_block", "Joining error pages");
        for (std::map<int, std::string>::iterator it = server.error_pages.begin(); it != server.error_pages.end(); it++)
            it->second = join_paths(server.server_root, it->second);
    }
    if (server.locations.size() > 0)
    {
        logger->log(LOG_DEBUG, "parse_server_block", "Joining locations");
        for (std::map<std::string, LocationConfig>::iterator it = server.locations.begin(); 
             it != server.locations.end(); ++it)
        {
            if (it->first.empty()) {
                logger->log(LOG_WARNING, "parse_server_block", "Empty location key found");
                continue;
            }            
            if (!it->first.empty() && it != server.locations.end() && it->second.loc_error_pages.size() > 0)
            {
                for (std::map<int, std::string>::iterator it2 = it->second.loc_error_pages.begin(); 
                     it2 != it->second.loc_error_pages.end(); ++it2)
                {
                    if (it2->second.empty() && it2 != it->second.loc_error_pages.end()) {
                        logger->log(LOG_WARNING, "parse_server_block", "Empty error page path found");
                        continue;
                    }

                    std::string temp_path = it2->second;
                    
                    if (!it->second.loc_root.empty() && it2 != it->second.loc_error_pages.end()) {
                        temp_path = join_paths(it->second.loc_root, temp_path);
                    }
                    
                    if (!server.server_root.empty() && it2 != it->second.loc_error_pages.end()) {
                        temp_path = join_paths(server.server_root, temp_path);
                    }
                    it2->second = temp_path;
                }
            }
        }
    }
    

    for (std::map<std::string, LocationConfig>::iterator it = server.locations.begin(); 
         it != server.locations.end(); ++it) {
        try {
            if (!it->second.loc_root.empty()) {
                if (it->second.loc_root.find("..") != std::string::npos) {
                    logger->fatal_log("parse_server_block", "Path traversal detectado en loc_root");
                }
                it->second.loc_root = join_paths(server.server_root, it->second.loc_root);
            }

            if (!it->first.empty() && !it->second.loc_root.empty()) {
                if (compare_paths(it->first, it->second.loc_root)) {
                    it->second.is_root = true;
                }
            }

            if (it->second.loc_error_pages.empty() && !server.error_pages.empty()) {
                it->second.loc_error_pages = server.error_pages;
            }
        }
        catch (const std::exception& e) {
            logger->fatal_log("parse_server_block", 
                std::string("Error processing location: ") + e.what());
        }
    }

    if (server.server_name == "")
        server.server_name = "localhost";
    if (server.client_max_body_size == 0)
        server.client_max_body_size = 52428800;

    if (check_obligatory_params(server, logger))
        logger->fatal_log("parse_server_block", "Obligatory parameters are not valid.");
    return server;
}

/**
 * @brief Parses all server configurations from the raw configuration lines.
 *
 * Processes the entire configuration file and extracts all server blocks.
 *
 * @param rawLines Vector of configuration file lines.
 * @param logger Pointer to the logger instance.
 * @return std::vector<ServerConfig> Vector of parsed server configurations.
 * @throw Logger::fatal_log if no valid servers are found or if duplicates exist.
 */
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
            if (!check_server_brackets(*it))
                logger->fatal_log("parse_servers", "Invalid server definition: " + *it);
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
    return servers;
}

/**
 * @brief Parses a configuration file and returns server configurations.
 *
 * Entry point for configuration file parsing.
 *
 * @param path Path to the configuration file.
 * @param logger Pointer to the logger instance.
 * @return std::vector<ServerConfig> Vector of parsed server configurations.
 * @throw Logger::fatal_log if file cannot be parsed or no servers are found.
 */
std::vector<ServerConfig> parse_file(std::string path, Logger* logger)
{
    logger->log(LOG_DEBUG, "parse_file", "Parsing file: " + path);
    std::vector<std::string> rawLines = get_raw_lines(path);
    logger->log(LOG_DEBUG, "parse_file", "Raw lines obtained: " + int_to_string(rawLines.size()));
    std::vector<ServerConfig> servers = parse_servers(rawLines, logger);
    if (servers.size() == 0)
        logger->fatal_log("parse_file", "No servers foundss");
    return servers;
}
