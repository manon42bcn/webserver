/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   file.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/15 23:50:49 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

void handleLocationBlock(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end) {
    std::map<std::string, std::string> locations;
    // Location location;

    // std::cout << "Location block found" << std::endl;
    for (std::vector<std::string>::iterator it = start; it != end; it++) {
        // std::cout << "Location block line: " <<RED << *it << RESET << std::endl;
        // if (it->find("location") != std::string::npos)
            // location.setPath(getValue(cleanLine(*it), "location"));



        if (it->find("}") != std::string::npos)
            break;
    }
    // std::cout << "Location block end" << std::endl;
    // for (std::map<std::string, std::string>::iterator it = locations.begin(); it != locations.end(); it++) {
    //     std::cout << "Location block: " << it->first << " - " << it->second << std::endl;
    // }
    // std::cout << location << std::endl;
    // return 
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

std::vector<std::string> split_default_pages(std::string default_pages)
{
    std::vector<std::string> default_pages_vector;
    std::istringstream iss(default_pages);
    std::string page;
    while (iss >> page)
        default_pages_vector.push_back(page);
    return default_pages_vector;
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

bool find_exact_string(const std::string& line, const std::string& str) {
    size_t pos = line.find(str);
    if (pos != std::string::npos) {
        if (pos > 0 && std::isalnum(line[pos - 1])) {
            return false;
        }
        if (pos + str.length() < line.length() && std::isalnum(line[pos + str.length()])) {
            return false;
        }
        return true;
    }
    return false;
}

std::map<int, std::string> split_error_pages(std::string error_pages)
{
    std::map<int, std::string> error_pages_map;
    std::istringstream iss(error_pages);
    std::string token;
    std::vector<int> error_codes;
    std::string path;

    while (iss >> token)
    {
        bool is_number = true;
        for (std::string::iterator it = token.begin(); it != token.end(); ++it)
        {
            if (!std::isdigit(*it))
            {
                is_number = false;
                break;
            }
        }
        if (is_number)
        {
            error_codes.push_back(std::atoi(token.c_str()));
        }
        else
        {
            path = token;
            break;
        }
    }

    for (std::vector<int>::iterator it = error_codes.begin(); it != error_codes.end(); ++it)
    {
        std::string adjusted_path = path;
        size_t x_pos = adjusted_path.find('x');
        if (x_pos != std::string::npos) {
            adjusted_path.replace(x_pos, 1, std::to_string(*it % 10));
        }
        error_pages_map[*it] = adjusted_path;
    }

    return error_pages_map;
}

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
    std::cout << YELLOW << "  Autoindex: " RESET << server.autoindex << std::endl;
    std::cout << YELLOW << "  Server root: " RESET << server.server_root << std::endl;
    std::cout << YELLOW << "  Default pages: " RESET << server.default_pages.size() << std::endl;
    if (server.default_pages.size() > 0)
    {
        for (std::vector<std::string>::iterator it = server.default_pages.begin(); it != server.default_pages.end(); it++)
        {
            std::cout << YELLOW << "    Default page: " RESET << *it << std::endl;
        }
    }
}

ServerConfig parseServerBlock(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end)
{
    ServerConfig server;
    std::string temp;
    int temp_int;

    for (std::vector<std::string>::iterator it = start; *it != *end; it++)
    {
        if (find_exact_string(*it, "location"))
        {
            std::vector<std::string>::iterator start_location = it;
            std::vector<std::string>::iterator end_location = findLocationBlockEnd(start_location, end);
            handleLocationBlock(start_location, end_location);
            it = end_location;
        }
        if (find_exact_string(*it, "port"))
        {
            temp_int = check_port(getValue(*it, "port"));
            if (temp_int > 0)
                server.port = temp_int;
            else
            {
                std::cout << "Error: port " << temp << " is not valid." << std::endl;
                exit(1);
            }
        }
        if (find_exact_string(*it, "server_name"))
        {
            if (!check_server_name(getValue(*it, "server_name")))
            {
                std::cout << "Error: server_name " << getValue(*it, "server_name") << " is not valid." << std::endl;
                exit(1);
            }
            server.server_name = getValue(*it, "server_name");
        }
        if (find_exact_string(*it, "error_page"))
        {

            if (check_error_page(getValue(*it, "error_page")))
            {
                server.error_pages = split_error_pages(getValue(*it, "error_page"));
            }
            else
            {
                std::cout << "Error: error_page " << getValue(*it, "error_page") << " is not valid." << std::endl;
                exit(1);
            }
        }
        if (find_exact_string(*it, "client_max_body_size"))
            server.client_max_body_size = getValue(*it, "client_max_body_size");
        if (find_exact_string(*it, "autoindex"))
        {
            if (getValue(*it, "autoindex") == "on")
                server.autoindex = true;
            else if (getValue(*it, "autoindex") == "off")
                server.autoindex = false;
            else
            {
                std::cout << "Error: autoindex " << getValue(*it, "autoindex") << " is not valid." << std::endl;
                exit(1);
            }
        }
        if (find_exact_string(*it, "root"))
            server.server_root = getValue(*it, "root");
        if (find_exact_string(*it, "index"))
        {
            if (!check_default_page(getValue(*it, "index")))
            {
                std::cout << "Error: default page " << getValue(*it, "index") << " is not valid." << std::endl;
                exit(1);
            }
            server.default_pages = split_default_pages(getValue(*it, "index"));
        }
    }

    if (server.server_name == "")
        server.server_name = "localhost";

    // if (!server.checkObligatoryParams())
    //     throw std::runtime_error("Obligatory parameters are missing in server block in server " + server.getServerName());
    //  verificar si tengo 2 veces el mismo parametro


    print_server_config(server);
    return server;
}

std::vector<std::string> get_raw_lines(std::string path)
{
    std::ifstream file(path);
    std::vector<std::string> rawLines;
    std::string line;

    while (getline(file, line))
    {
        line = cleanLine(line);
        if (line.empty())
            continue;
        rawLines.push_back(line);
    }
    file.close();
    return rawLines;
}

std::vector<ServerConfig> parse_servers(std::vector<std::string> rawLines)
{
    std::vector<std::string>::iterator start;
    std::vector<std::string>::iterator end;
    std::vector<ServerConfig> servers;
    for (std::vector<std::string>::iterator it = rawLines.begin(); it != rawLines.end(); it++)
    {
        if (it->find("server") != std::string::npos && it->find("server_name") == std::string::npos)
        {
            start = it;
            end = findServerBlockEnd(start, rawLines.end());

            servers.push_back(parseServerBlock(start, end));

            end = findServerBlockEnd(start, rawLines.end());
            // for (std::vector<std::string>::iterator it = start; it != rawLines.end(); it++) {
            //     if (it->find("}") != std::string::npos) {
            //         end = it;
            //         server = parseServerBlock(start, end);
            //         servers.push_back(server);
            //         break;
            //     }
            // }
        }
    }
    return servers;
}

void parse_file(std::string path)
{
    std::vector<std::string> rawLines = get_raw_lines(path);
    std::vector<ServerConfig> servers = parse_servers(rawLines);
}
