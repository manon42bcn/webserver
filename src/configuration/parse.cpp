/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 18:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/06 20:09:13 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Server.hpp"
#include "Location.hpp"

Location Config::handleLocationBlock(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end) {
    std::map<std::string, std::string> locations;
    Location location;

    for (std::vector<std::string>::iterator it = start; it != end; it++) {
        if (it->find("location") != std::string::npos)
            location.setPath(getValue(*it, "location"));
        if (it->find("path") != std::string::npos)
            location.setPath(getValue(*it, "path"));
        if (it->find("index") != std::string::npos)
            location.setIndexFile(getValue(*it, "index"));
        if (it->find("root") != std::string::npos)
            location.setRoot(getValue(*it, "root"));
        if (it->find("autoindex") != std::string::npos)
        {
            if (getValue(*it, "autoindex") == "on") {
                location.setAutoIndex(true);
            }
            else {
                location.setAutoIndex(false);
            }
        }
        if (it->find("allowedMethods") != std::string::npos)
            location.setAllowedMethods(getValue(*it, "allowedMethods"));
        if (it->find("}") != std::string::npos)
            break;
    }
    
    return location;
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
        {
            Location location = handleLocationBlock(it, end);
            server.setLocations(location.getPath(), location);
            server.sumNumLocations();
        }
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
            servers.push_back(server);
            it = end;
        }
    }

    // for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it) {
    //     std::cout << RED << "Server: " << DEF_COLOR << it->getServerName() << std::endl;
    //     for (std::map<std::string, Location>::const_iterator locIt = it->_locations.begin(); locIt != it->_locations.end(); ++locIt) {
    //         std::cout << RED << "Location: " << DEF_COLOR << locIt->first <<std::endl;
    //     }
    //     std::cout << std::endl;
    // }
        
    setServers(servers);
}
