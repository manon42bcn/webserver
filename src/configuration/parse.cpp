/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 18:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/06 11:57:49 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Server.hpp"
#include "Location.hpp"

void Config::handleLocationBlock(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end) {
    std::map<std::string, std::string> locations;
    Location location;

    std::cout << "Location block found" << std::endl;
    for (std::vector<std::string>::iterator it = start; it != end; it++) {
        std::cout << "Location block line: " <<RED << *it << DEF_COLOR << std::endl;
        if (it->find("location") != std::string::npos)
            location.setPath(getValue(cleanLine(*it), "location"));



        if (it->find("}") != std::string::npos)
            break;
    }
    std::cout << "Location block end" << std::endl;
    // for (std::map<std::string, std::string>::iterator it = locations.begin(); it != locations.end(); it++) {
    //     std::cout << "Location block: " << it->first << " - " << it->second << std::endl;
    // }
    std::cout << location << std::endl;
}

Server Config::parseServerBlock(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end) {
    Server server;

    for (std::vector<std::string>::iterator it = start; *it != *end; it++) {
        if (it->find("host") != std::string::npos)
            server.setHost(getValue(*it, "host"));
        if (it->find("port") != std::string::npos)
            server.setPort(getValue(*it, "port"));
        if (it->find("server_name") != std::string::npos)
            server.setServerName(getValue(*it, "server_name"));
        if (it->find("error_page") != std::string::npos)
            server.setErrorPage(getValue(*it, "error_page"));
        if (it->find("client_max_body_size") != std::string::npos)
            server.setClientMaxBodySize(getValue(*it, "client_max_body_size"));
        if (it->find("autoindex") != std::string::npos)
            server.setAutoindex(getValue(*it, "autoindex"));
        if (it->find("location") != std::string::npos)
            handleLocationBlock(it, end);
        if (it->find("root") != std::string::npos)
            server.setRoot(getValue(*it, "root"));
    }

    if (server.getServerName() == "")
        server.setServerName("Default server name " + std::to_string(this->getNumServers()));
 
    // if (!server.checkObligatoryParams())
    //     throw std::runtime_error("Obligatory parameters are missing in server block in server " + server.getServerName());

    _numServers++;
    return server;
}

std::vector<std::string>::iterator findServerBlockEnd(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end) {
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

void Config::parseConfigFile(const std::string &filePath) {
    _configFilePath = filePath;
    std::ifstream file(_configFilePath);
    std::vector<std::string> rawLines;
    std::string line;
    
    while (getline(file, line)) {
        line = cleanLine(line);
        if (line.empty())
            continue;
        rawLines.push_back(line);
    }

    std::vector<std::string>::iterator start;
    std::vector<std::string>::iterator end;
    Server server;
    std::vector<Server> servers = getServers();
    for (std::vector<std::string>::iterator it = rawLines.begin(); it != rawLines.end(); it++) {
        if (it->find("server") != std::string::npos && it->find("server_name") == std::string::npos) {
            start = it;
            end = findServerBlockEnd(start, rawLines.end());

            server = parseServerBlock(start, end);

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
    setServers(servers);
}
