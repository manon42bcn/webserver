/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Configurationfile.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 18:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/03/11 20:33:13 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigurationFile.hpp"

ConfigFile::ConfigFile(const std::string& filePath) : configFilePath(filePath) {
    parseConfigFile();
}

ConfigFile::~ConfigFile() {
    std::cout << "ConfigFile is destroyed." << std::endl;
}

void ConfigFile::parseConfigFile() {
    std::cout << "parseConfigFile: " << this->configFilePath << std::endl; 
}

std::string ConfigFile::getValue(const std::string& key) {
    if (configMap.find(key) != configMap.end()) {
        return configMap[key];
    }
    return "";
}

