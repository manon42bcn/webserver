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

class Config {

public:

    Config(const std::string& filePath);
    ~Config();
    std::string getValue(const std::string& key);
    
private:
    std::map<std::string, std::string> configMap;
    std::string configFilePath;

    void parseConfigFile();
    std::string cleanLine(std::string line);

    struct MainContext {
    std::map<std::string, std::string> mainContext;
    };

    MainContext mainContext;

};

#endif