/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/21 23:13:21 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

std::string clean_line(std::string line)
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

std::string get_value(std::string line, const std::string& key) {
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

bool find_exact_string(const std::string& line, const std::string& str) {
    size_t pos = line.find(str);
    if (pos != std::string::npos) {
        if (pos > 0 && std::isalnum(line[pos - 1])) {
            return false;
        }
        if (pos + str.length() < line.length() && std::isalnum(line[pos + str.length()])) {
            return false;
        }
        return true;
    }
    return false;
}

std::vector<std::string> split_default_pages(std::string default_pages)
{
    std::vector<std::string> default_pages_vector;
    std::istringstream iss(default_pages);
    std::string page;
    while (iss >> page)
        default_pages_vector.push_back(page);
    return default_pages_vector;
}

std::string delete_first_slash(std::string path)
{
    if (path[0] == '/')
        path.erase(0, 1);
    return path;
}

std::map<int, std::string> split_error_pages(std::string error_pages)
{
    std::map<int, std::string> error_pages_map;
    std::istringstream iss(error_pages);
    std::string token;
    std::vector<int> error_codes;
    std::string path;
    std::string base_path = getenv("WEBSERVER_PATH");

    while (iss >> token)
    {
        bool is_number = true;
        for (std::string::iterator it = token.begin(); it != token.end(); ++it)
        {
            if (!std::isdigit(*it))
            {
                is_number = false;
                break;
            }
        }
        if (is_number)
        {
            error_codes.push_back(std::atoi(token.c_str()));
        }
        else
        {
            path = delete_first_slash(token);
            break;
        }
    }

    for (std::vector<int>::iterator it = error_codes.begin(); it != error_codes.end(); ++it)
    {
        std::string adjusted_path = path;
        size_t x_pos = adjusted_path.find('x');
        if (x_pos != std::string::npos) {
            std::stringstream ss;
            ss << (*it % 10);
            adjusted_path.replace(x_pos, 1, ss.str());
        }
        error_pages_map[*it] = base_path + adjusted_path;
    }

    return error_pages_map;
}

std::vector<std::string> get_raw_lines(std::string path)
{
    std::ifstream file(path.c_str());
    std::vector<std::string> rawLines;
    std::string line;

    while (getline(file, line))
    {
        line = clean_line(line);
        if (line.empty())
            continue;
        rawLines.push_back(line);
    }
    file.close();
    return rawLines;
}

std::string delete_brackets_clean(std::string line)
{
    std::string::size_type start_pos = line.find("{");
    std::string::size_type end_pos = line.find("}");
    if (start_pos != std::string::npos)
        line.erase(start_pos, 1);
    if (end_pos != std::string::npos)
        line.erase(end_pos, 1);
    line = clean_line(line);
    return line;
}
