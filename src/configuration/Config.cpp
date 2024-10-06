/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 18:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/03/23 18:00:22 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config() { }

Config::Config(const std::string& filePath) : _configFilePath(filePath) {
    parseConfigFile(filePath);
}

Config::~Config() { }

void Config::setServers(std::vector<Server> servers) {
    _servers = servers;
}

std::vector<Server> Config::getServers() const {
    return _servers;
}

int Config::getNumServers() const {
    return _numServers;
}

std::string Config::getConfigFilePath() const {
    return _configFilePath;
}

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

std::ostream& operator<<(std::ostream& os, const Config& config) {
    os << YELLOW << "Config file path: " << DEF_COLOR << config.getConfigFilePath() << std::endl;
    os << YELLOW << "Number of servers: " << DEF_COLOR << config.getNumServers() << std::endl;
    if (config.getNumServers() > 0) {
        os << YELLOW << "Servers: " << DEF_COLOR << std::endl;
        const std::vector<Server>& servers = config.getServers();
        for (std::vector<Server>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
            os << *it << std::endl;
        }
    }
    return os;
}
