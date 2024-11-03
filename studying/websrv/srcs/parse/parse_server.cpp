/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/28 19:01:48 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

void parse_location(std::vector<std::string>::iterator& it, std::vector<std::string>::iterator end, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing location block");
    
    std::string location_path = get_location_path(*it);
    LocationConfig location = parse_location_block(it, find_block_end(it, end), logger);
    
    if (location_path == "/") {
        location.loc_root = "";
    }
    
    server.locations[location_path] = location;
    it = skip_block(it, find_block_end(it, end));
}


void parse_template_error_page(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing template error page");
    server.template_error_page = get_value(*it, "template_error_page");
}


void parse_port(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing port");
    int port = check_port(get_value(*it, "port"));
    if (port != -1)
        server.port = port;
    else
        logger->fatal_log("parse_server_block", "Port " + get_value(*it, "port") + " is not valid.");
}

void parse_server_name(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing server name");
    std::string serverName = get_value(*it, "server_name");
    if (check_server_name(serverName))
        server.server_name = serverName;
    else
        logger->fatal_log("parse_server_block", "Server name " + serverName + " is not valid.");
}

void parse_root(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing server root");
    std::string root = get_value(*it, "root");
    if (check_root(root))
        server.server_root = join_paths(get_server_root(), get_value(*it, "root"));
    else
        logger->fatal_log("parse_server_block", "Server root " + root + " is not valid.");
}

void parse_index(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing default pages");
    if (check_default_page(get_value(*it, "index")))
        server.default_pages = split_default_pages(get_value(*it, "index"));
    else
        logger->fatal_log("parse_server_block", "Default page " + get_value(*it, "index") + " is not valid.");
}

void parse_client_max_body_size(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing client max body size");
    std::string client_max_body_size = get_value(*it, "client_max_body_size");
    if (check_client_max_body_size(client_max_body_size))
        server.client_max_body_size = client_max_body_size;
    else
        logger->fatal_log("parse_server_block", "Client max body size " + client_max_body_size + " is not valid.");
}

void parse_error_page(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing error page");
    std::string error_page = get_value(*it, "error_page");
    if (check_error_page(error_page))
    {
        try {
            logger->log(LOG_DEBUG, "parse_server_block", "Splitting error pages");
            std::map<int, std::string> new_error_pages = split_error_pages(error_page);
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

void parse_error_mode(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing error mode");
    if (check_error_mode(get_value(*it, "error_mode")))
        server.error_mode = string_to_error_mode(get_value(*it, "error_mode"));
    else
        logger->fatal_log("parse_server_block", "Error mode " + get_value(*it, "error_mode") + " is not valid.");
}


