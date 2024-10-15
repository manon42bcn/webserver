/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/15 23:45:00 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

std::string cleanLine(std::string line)
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

std::string getValue(std::string line, const std::string& key) {
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