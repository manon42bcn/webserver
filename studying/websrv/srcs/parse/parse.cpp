/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/28 19:01:48 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"
#include <set>

std::vector<t_allowed_methods> parse_limit_except(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end, Logger* logger) {
    std::vector<t_allowed_methods> all_methods;
    std::vector<t_allowed_methods> methods;

    std::vector<std::string>::iterator start_copy = start;

    for (int i = 0; i < 4; i++)
        all_methods.push_back(static_cast<t_allowed_methods>(i));

    logger->log(LOG_DEBUG, "parse_limit_except", "Parsing limit except block");

    std::string methods_line = start_copy->substr(12);
    std::istringstream iss(methods_line);
    std::vector<std::string> method_tokens;
    std::string token;
    while (iss >> token) {
        method_tokens.push_back(token);
    }
    for (std::vector<std::string>::iterator it = method_tokens.begin(); it != method_tokens.end(); it++) {
        methods.push_back(string_to_method(*it));
    }
    start_copy++;
    for (std::vector<std::string>::iterator it = start_copy; it != end; it++) {
        if (find_exact_string(*it, "deny"))
        {
            for (std::vector<t_allowed_methods>::iterator mit = methods.begin(); mit != methods.end(); mit++) {
                for (std::vector<t_allowed_methods>::iterator ait = all_methods.begin(); ait != all_methods.end(); ait++) {
                    if (*mit == *ait) {
                        all_methods.erase(ait);
                    }
                }
            }
        }
    }

    return all_methods;
}

LocationConfig parse_location_block(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end, Logger* logger) {
    std::map<std::string, std::string> locations;
    LocationConfig location;
    location.cgi = false;
    location.loc_root = "";


    start++;
    for (std::vector<std::string>::iterator it = start; it != end; it++) {
        if (find_exact_string(*it, "index"))
        {
            if (check_default_page(get_value(*it, "index")))
                location.loc_default_pages = split_default_pages(get_value(*it, "index"));
            else
                logger->fatal_log("parse_location_block", "Default page " + get_value(*it, "index") + " is not valid.");
        }
        else if (find_exact_string(*it, "error_page"))
        {
            std::string error_page = get_value(*it, "error_page");
            if (check_error_page(error_page))
            {
                std::map<int, std::string> new_error_pages = split_error_pages(error_page);
                location.loc_error_pages.insert(new_error_pages.begin(), new_error_pages.end());
            }
            else
                logger->fatal_log("parse_location_block", "Error page " + error_page + " is not valid.");
        }
        else if (find_exact_string(*it, "root"))
        {
            std::string root = get_value(*it, "root");
            if (check_root(root))
                location.loc_root = join_paths(get_server_root(), get_value(*it, "root"));
            else
                logger->fatal_log("parse_location_block", "Root " + root + " is not valid.");
        }
        else if (find_exact_string(*it, "autoindex"))
        {   
            if (check_autoindex(get_value(*it, "autoindex")))
            {
                if (get_value(*it, "autoindex") == "on")
                    location.autoindex = true;
                else if (get_value(*it, "autoindex") == "off")
                    location.autoindex = false;
            }
            else
                logger->fatal_log("parse_location_block", "Autoindex " + get_value(*it, "autoindex") + " is not valid.");
        }
        else if (find_exact_string(*it, "limit_except"))
        {
            location.loc_allowed_methods = parse_limit_except(it, find_block_end(it, end), logger);
            it = skip_block(it, find_block_end(it, end));
            if (it == end)
                break;
        }
        else if (find_exact_string(*it, "cgi"))
        {
            if (check_cgi(get_value(*it, "cgi")))
            {
                if (get_value(*it, "cgi") == "on")
                    location.cgi = true;
                else if (get_value(*it, "cgi") == "off")
                    location.cgi = false;
            }
            else
                logger->fatal_log("parse_location_block", "CGI " + get_value(*it, "cgi") + " is not valid.");
        }
        else if (it->find("}") != std::string::npos)
        {
            std::cout << RED << "Saliendo porque encontre } en location block" << RESET << std::endl;
            // return location;
            break;
        }
        else
        {
            std::cout << RED << "Error: [" << *it << "] is not valid parameter in location block" << RESET << std::endl;
            // std::cout << RED << "Get value: " << find_exact_string(*it, "location") << RESET << std::endl;
            continue;
        }
    }

    if (location.loc_allowed_methods.empty()) {
        location.loc_access = ACCESS_FORBIDDEN; 
    } else {
        bool has_write = false;
        bool has_delete = false;
        
        for (std::vector<t_allowed_methods>::iterator it = location.loc_allowed_methods.begin(); 
             it != location.loc_allowed_methods.end(); ++it) {
            if (*it == POST || *it == PUT) {
                has_write = true;
            } else if (*it == DELETE) {
                has_delete = true;
            }
        }
        
        if (has_delete) {
            location.loc_access = ACCESS_DELETE;
        } else if (has_write) {
            location.loc_access = ACCESS_WRITE;
        } else {
            location.loc_access = ACCESS_READ;
        }
    }

    // Obligatorios
    if (location.loc_root == "")
        logger->fatal_log("parse_location_block", "Location root is not valid.");

    return location;
}

ServerConfig parse_server_block(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end, Logger* logger)
{
    ServerConfig server;
    server.port = -42;
    server.server_root = "";
    
    start++;
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing server block");
    for (std::vector<std::string>::iterator it = start; it != end; ++it)
    {
        // std::cout << BLUE << "Server block: " << *it << RESET << std::endl;
        if (find_exact_string(*it, "location"))
        {
            logger->log(LOG_DEBUG, "parse_server_block", "Parsing location block");
            std::string location_path = get_location_path(*it);
            LocationConfig location = parse_location_block(it, find_block_end(it, end), logger);
            // Hardcodeado
            if (location_path == "/")
                location.loc_root = "";
            server.locations[location_path] = location;
            it = skip_block(it, find_block_end(it, end));
            // std::cout << RED << "Terminando location block en server con valor " << *it << RESET << std::endl;
        }
        else if (find_exact_string(*it, "template_error_page"))
        {
            logger->log(LOG_DEBUG, "parse_server_block", "Parsing template error page");
            server.template_error_page = get_value(*it, "template_error_page");
        }
        else if (find_exact_string(*it, "port"))
        {
            logger->log(LOG_DEBUG, "parse_server_block", "Parsing port");
            int port = check_port(get_value(*it, "port"));
            if (port != -1)
                server.port = port;
            else
                logger->fatal_log("parse_server_block", "Port " + get_value(*it, "port") + " is not valid.");
        }
        else if (find_exact_string(*it, "server_name"))
        {
            logger->log(LOG_DEBUG, "parse_server_block", "Parsing server name");
            std::string serverName = get_value(*it, "server_name");
            if (check_server_name(serverName))
                server.server_name = serverName;
            else
                logger->fatal_log("parse_server_block", "Server name " + serverName + " is not valid.");
        }
        else if (find_exact_string(*it, "root"))
        {
            logger->log(LOG_DEBUG, "parse_server_block", "Parsing server root");
            std::string root = get_value(*it, "root");
            if (check_root(root))
                server.server_root = join_paths(get_server_root(), get_value(*it, "root"));
            else
                logger->fatal_log("parse_server_block", "Server root " + root + " is not valid.");
        }
        else if (find_exact_string(*it, "index"))
        {
            logger->log(LOG_DEBUG, "parse_server_block", "Parsing default pages");
            if (check_default_page(get_value(*it, "index")))
                server.default_pages = split_default_pages(get_value(*it, "index"));
            else
                logger->fatal_log("parse_server_block", "Default page " + get_value(*it, "index") + " is not valid.");
        }
        else if (find_exact_string(*it, "client_max_body_size"))
        {
            logger->log(LOG_DEBUG, "parse_server_block", "Parsing client max body size");
            std::string client_max_body_size = get_value(*it, "client_max_body_size");
            if (check_client_max_body_size(client_max_body_size))
                server.client_max_body_size = get_value(*it, "client_max_body_size");
            else
                logger->fatal_log("parse_server_block", "Client max body size " + client_max_body_size + " is not valid.");
        }
        else if (find_exact_string(*it, "error_page")) // Solucion temporal
        {
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
                    logger->log(LOG_ERROR, "parse_server_block", "Error splitting error pages: " + std::string(e.what()));
                    continue;
                }
            }
            else
                logger->fatal_log("parse_server_block", "Error page " + error_page + " is not valid.");
        }
        else if (find_exact_string(*it, "autoindex"))
        {
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
        else if (find_exact_string(*it, "error_mode"))
        {
            logger->log(LOG_DEBUG, "parse_server_block", "Parsing error mode");
            if (check_error_mode(get_value(*it, "error_mode")))
                server.error_mode = string_to_error_mode(get_value(*it, "error_mode"));
            else
                logger->fatal_log("parse_server_block", "Error mode " + get_value(*it, "error_mode") + " is not valid.");
        }
        else if (it->find("}") != std::string::npos)
        {
            std::cout << RED << "Saliendo porque encontre } en server block" << RESET << std::endl;
            // deberia ser break o continue?
            break;
        }
        else
        {
            std::cout << RED << "Error: " << *it << " is not valid parameter in server block" << RESET << std::endl;
            continue;
        }
    }

    // Configuracion por defecto
    if (server.server_name == "")
        server.server_name = "localhost";
    if (!server.autoindex)
        server.autoindex = false;
    server.ws_root = get_server_root();
    if (server.error_pages.size() > 0)
    {
        for (std::map<int, std::string>::iterator it = server.error_pages.begin(); it != server.error_pages.end(); it++)
            it->second = join_paths(server.server_root, it->second);
    }
    // Obligatorios
    if (server.port == -42)
        logger->fatal_log("parse_server_block", "Port is not valid.");
    if (server.server_root == "")
        logger->fatal_log("parse_server_block", "Server root is not valid.");
    

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
    //print_servers(servers);
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
