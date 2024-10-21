/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   file.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/22 00:06:16 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"
#include <set>

LocationConfig handleLocationBlock(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end) {
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
            LocationConfig location = handleLocationBlock(it, end);
            std::vector<std::string>::iterator start_location = it;
            std::vector<std::string>::iterator end_location = findLocationBlockEnd(start_location, end);
            location = handleLocationBlock(start_location, end_location);
            server.locations["/"] = location;
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
        {
            std::string base_path = getenv("WEBSERVER_PATH");
           server.server_root = base_path + delete_first_slash(get_value(*it, "root"));
        }
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


    // print_server_config(server);
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
            // std::cout << "Server block found" << std::endl;
            start = it;
            end = findServerBlockEnd(start, rawLines.end());
            servers.push_back(parseServerBlock(start, end));
            it = skip_block(start, end);
        }
    }

    // std::cout << "Servers found: " << servers.size() << std::endl;
    return servers;
}


std::vector<ServerConfig> parse_file(std::string path)
{
    const Logger*       _log;
    _log = new Logger(LOG_DEBUG, false);

    _log->log(LOG_DEBUG, "parse_file", "Parsing file: " + path);
    std::vector<std::string> rawLines = get_raw_lines(path);
    _log->log(LOG_DEBUG, "parse_file", "Raw lines: " + int_to_string(rawLines.size()));
    std::vector<ServerConfig> servers = parse_servers(rawLines);
    _log->log(LOG_DEBUG, "parse_file", "Servers found: " + int_to_string(servers.size()));

    return servers;
}



