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

Config::Config(const std::string& filePath) : configFilePath(filePath) {
    parseConfigFile();
}

Config::~Config() {
    std::cout << "ConfigFile is destroyed." << std::endl;
}

std::string Config::getValue(const std::string& key) {
    if (configMap.find(key) != configMap.end()) {
        return configMap[key];
    }
    return "";
}

