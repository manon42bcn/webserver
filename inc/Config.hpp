/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 18:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/03/23 17:59:53 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_H
#define CONFIG_H
#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include "webserver.hpp"
#include "Server.hpp"

class Config
{

public:

    Config();
    Config(const std::string &filePath); // Necesary (?)
    ~Config();

    void parseConfigFile(const std::string &filePath);

    std::string getConfigFilePath() const;

    void setServers(std::vector<Server> servers);
    std::vector<Server> getServers() const;
    int getNumServers() const;
    
    std::string getValue(std::string line, const std::string &key);

    void handleLocationBlock(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end);

private:

    std::vector<Server> _servers;
    std::string _configFilePath;
    int _numServers;
    std::map<std::string, std::string> _locations;

    Server parseServerBlock(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end);

    std::string cleanLine(std::string line);
};

std::ostream &operator<<(std::ostream &os, const Config &config);

#endif
