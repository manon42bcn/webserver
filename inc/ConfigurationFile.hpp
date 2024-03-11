/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configurationfile.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 18:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/03/11 20:21:04 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGFILE_H
#define CONFIGFILE_H
#include <iostream>
#include <map>
#include <fstream>
#include <sstream>

class ConfigFile {

public:

    ConfigFile(const std::string& filePath);
    ~ConfigFile();
    std::string getValue(const std::string& key);
    
private:
    std::map<std::string, std::string> configMap;
    std::string configFilePath;

    void parseConfigFile();

};

#endif