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
