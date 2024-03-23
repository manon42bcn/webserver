/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 18:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/03/23 20:44:53 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

std::string Config::cleanLine(std::string line)
{
    std::string::size_type start_pos = line.find_first_not_of(" \t");
    if (start_pos != std::string::npos) {
        line.erase(0, start_pos);
    }

    std::string::size_type end_pos = line.find_last_not_of(" \t");
    if (end_pos != std::string::npos) {
        line.erase(end_pos + 1, line.length() - end_pos - 1);
    }

    if (!line.empty() && line[line.size() - 1] == ';') {
        line.erase(line.size() - 1);
    }

    if (line.empty() || line[0] == '#' || line[0] == '{' || line[0] == '}') { // || line.back() == '{'
        return "";
    }
    return line;
}

void Config::parseConfigFile() {
    std::cout << "parseConfigFile: " << this->configFilePath << std::endl; 

    std::ifstream file(configFilePath);
    std::string line;
    while (getline(file, line)) {

        line = cleanLine(line);

        if (line.empty())
            continue;

        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss >> std::ws, key, ' ') && std::getline(iss >> std::ws, value))
        {
            configMap[key] = value;
            if (value == "{")
                std::cout << key << std::endl;
            else
                std::cout << "Key: [" << key << "], Value: [" << value << "]" << std::endl;
        }
    }
}