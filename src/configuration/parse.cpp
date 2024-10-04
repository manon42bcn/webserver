/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 18:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/04 16:33:17 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Server.hpp"

std::string Config::cleanLine(std::string line)
{
    std::string::size_type start_pos = line.find_first_not_of(" \t");
    if (start_pos != std::string::npos)
        line.erase(0, start_pos);

    std::string::size_type end_pos = line.find_last_not_of(" \t");

    if (end_pos != std::string::npos)
        line.erase(end_pos + 1, line.length() - end_pos - 1);
    if (!line.empty() && line[line.size() - 1] == ';')
        line.erase(line.size() - 1);
    if (line.empty() || line[0] == '#')
        return "";

    return line;
}

std::string Config::getValue(std::string line, const std::string& key) {
    std::string::size_type keyPos = line.find(key);
    if (keyPos != std::string::npos) {
        std::string::size_type valueStart = line.find_first_not_of(" \t", keyPos + key.length());
        if (valueStart != std::string::npos) {
            std::string::size_type valueEnd = line.find_last_not_of(" \t");
            if (valueEnd != std::string::npos && valueEnd >= valueStart) {
                return line.substr(valueStart, valueEnd - valueStart + 1);
            }
        }
    }
    return "";
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
    }

    if (server.getServerName() == "")
        server.setServerName("Default server name " + std::to_string(this->getNumServers()));
        
    // if (!server.checkObligatoryParams())
    //     throw std::runtime_error("Obligatory parameters are missing in server block in server " + server.getServerName());
    
    _numServers++;
    return server;
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
            for (std::vector<std::string>::iterator it = rawLines.begin(); it != rawLines.end(); it++) {
                if (it->find("}") != std::string::npos) {
                    end = it;
                    server = parseServerBlock(start, end);
                    servers.push_back(server);
                    break;
                }
            }
        }
    }
    setServers(servers);
}
