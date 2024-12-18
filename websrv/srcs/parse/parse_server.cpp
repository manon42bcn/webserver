/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_server.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/11/17 14:22:52 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

/**
 * @brief Parses a location directive within a server block.
 *
 * @param it Current iterator position in configuration.
 * @param end Iterator to end of configuration.
 * @param logger Pointer to logger instance.
 * @param server Reference to server configuration being built.
 * @throw Logger::fatal_log if location path is invalid.
 */
void parse_location(std::vector<std::string>::iterator& it, std::vector<std::string>::iterator end, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing location block");
    
    std::string location_path = get_location_path(*it);
    if (location_path.find(" ") != std::string::npos
        || location_path.find("\t") != std::string::npos
        || location_path.find("\n") != std::string::npos
        || location_path.find("\r") != std::string::npos)
        logger->fatal_log("parse_server_block", "Location path " + location_path + " contains spaces");
    if (location_path[0] != '/')
        logger->fatal_log("parse_server_block", "Location path " + location_path + " does not start with /");
    LocationConfig location = parse_location_block(it, find_block_end(it, end), logger);
    
    if (check_duplicate_location(location_path, server.locations))
    {
        logger->fatal_log("parse_server_block", 
            "Location duplicated: " + location_path);
        std::cout << RED << "Error: Location duplicated: " << location_path << RESET << std::endl;
    }

    server.locations[location_path] = location;
    it = skip_block(it, find_block_end(it, end));
}

/**
 * @brief Parses a template error page directive within a server block.
 *
 * @param it Current iterator position in configuration.
 * @param logger Pointer to logger instance.
 * @param server Reference to server configuration being built.
 */
void parse_template_error_page(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing template error page");
    server.template_error_page = get_value(*it, "template_error_page");
}

/**
 * @brief Parses a port directive within a server block.
 *
 * @param it Current iterator position in configuration.
 * @param logger Pointer to logger instance.
 * @param server Reference to server configuration being built.
 * @throw Logger::fatal_log if port is invalid or already defined.
 */
void parse_port(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    if (server.port != -42)
        logger->fatal_log("parse_server_block", "Double port definition");
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing port");
    int port = check_port(get_value(*it, "port"));
    if (port != -1)
        server.port = port;
    else
        logger->fatal_log("parse_server_block", "Port " + get_value(*it, "port") + " is not valid.");
}

/**
 * @brief Parses a server name directive within a server block.
 *
 * @param it Current iterator position in configuration.
 * @param logger Pointer to logger instance.
 * @param server Reference to server configuration being built.
 * @throw Logger::fatal_log if server name is invalid.
 */
void parse_server_name(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing server name");
    std::string serverName = get_value(*it, "server_name");
    if (check_server_name(serverName))
        server.server_name = serverName;
    else
        logger->fatal_log("parse_server_block", "Server name " + serverName + " is not valid.");
}

/**
 * @brief Parses a root directive within a server block.
 *
 * @param it Current iterator position in configuration.
 * @param logger Pointer to logger instance.
 * @param server Reference to server configuration being built.
 * @throw Logger::fatal_log if root is invalid or already defined.
 */
void parse_root(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing server root");
    if (server.server_root != "")
        logger->fatal_log("parse_server_block", "Double root definition");
    std::string root = get_value(*it, "root");
    if (check_root(root))
        server.server_root = join_paths(get_server_root(), get_value(*it, "root"));
    else
        logger->fatal_log("parse_server_block", "Server root " + root + " is not valid.");
}

/**
 * @brief Parses an index directive within a server block.
 *
 * @param it Current iterator position in configuration.
 * @param logger Pointer to logger instance.
 * @param server Reference to server configuration being built.
 * @throw Logger::fatal_log if default page is invalid.
 */
void parse_index(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing default pages");
    if (it->find("error_page") != std::string::npos)
        return;
    if (check_default_page(get_value(*it, "index")))
        server.default_pages = split_default_pages(get_value(*it, "index"));
    else
        logger->fatal_log("parse_server_block", "Default page " + get_value(*it, "index") + " is not valid.");

}

/**
 * @brief Parses a client max body size directive within a server block.
 *
 * @param it Current iterator position in configuration.
 * @param logger Pointer to logger instance.
 * @param server Reference to server configuration being built.
 * @throw Logger::fatal_log if client max body size is invalid.
 */
void parse_client_max_body_size(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing client max body size");
    std::string client_max_body_size = get_value(*it, "client_max_body_size");
    if (check_client_max_body_size(client_max_body_size))
        server.client_max_body_size = string_to_bytes(client_max_body_size);
    else
        logger->fatal_log("parse_server_block", "Client max body size " + client_max_body_size + " is not valid.");
}

/**
 * @brief Parses an error page directive within a server block.
 *
 * @param it Current iterator position in configuration.
 * @param logger Pointer to logger instance.
 * @param server Reference to server configuration being built.
 * @throw Logger::fatal_log if error page configuration is invalid or duplicated.
 */
void parse_error_page(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing error page");
    std::string error_page = get_value(*it, "error_page");
    if (check_error_page(error_page))
    {
        try {
            logger->log(LOG_DEBUG, "parse_server_block", "Splitting error pages");
            std::map<int, std::string> new_error_pages = split_error_pages(error_page);
            for (std::map<int, std::string>::iterator it = server.error_pages.begin(); it != server.error_pages.end(); it++)
            {
                if (it->first == new_error_pages.begin()->first)
                    logger->fatal_log("parse_server_block", "Redefinition of error page number in server block");
            }
            logger->log(LOG_DEBUG, "parse_server_block", "Inserting error pages");
            server.error_pages.insert(new_error_pages.begin(), new_error_pages.end());
            logger->log(LOG_DEBUG, "parse_server_block", "Error pages inserted");
        } catch (const std::exception& e) {
            logger->fatal_log("parse_server_block", "Error splitting error pages: " + std::string(e.what()));
        }
    }
    else
        logger->fatal_log("parse_server_block", "Error page " + error_page + " is not valid.");
}

/**
 * @brief Parses an autoindex directive within a server block.
 *
 * @param it Current iterator position in configuration.
 * @param logger Pointer to logger instance.
 * @param server Reference to server configuration being built.
 * @throw Logger::fatal_log if autoindex value is invalid.
 */
void parse_autoindex(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing autoindex");
    if (check_autoindex(get_value(*it, "autoindex")))
    {
        if (get_value(*it, "autoindex") == "on")
            server.autoindex = true;
        else if (get_value(*it, "autoindex") == "off")
            server.autoindex = false;
    }
    else
        logger->fatal_log("parse_server_block", "Autoindex " + get_value(*it, "autoindex") + " is not valid.");
}

/**
 * @brief Parses an error mode directive within a server block.
 *
 * @param it Current iterator position in configuration.
 * @param logger Pointer to logger instance.
 * @param server Reference to server configuration being built.
 * @throw Logger::fatal_log if error mode is invalid.
 */
void parse_error_mode(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing error mode");
    if (check_error_mode(get_value(*it, "error_mode")))
        server.error_mode = string_to_error_mode(get_value(*it, "error_mode"));
    else
        logger->fatal_log("parse_server_block", "Error mode " + get_value(*it, "error_mode") + " is not valid.");
}
