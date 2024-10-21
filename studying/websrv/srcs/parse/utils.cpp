/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/22 00:00:39 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

/**
 * @brief Cleans a line by trimming whitespace, removing a trailing semicolon,
 * and returning an empty string if the line is empty or starts with '#'.
 *
 * @param line The line to be cleaned.
 * @return A cleaned version of the line.
 */
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

/**
 * @brief Extracts the value associated with a key in a given line.
 *
 * @param line The line containing the key-value pair.
 * @param key The key to search for.
 * @return The value associated with the key, or an empty string if not found.
 */
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

/**
 * @brief Searches for an exact string in a line, ensuring it is not
 * preceded or followed by alphanumeric characters.
 *
 * @param line The line to search within.
 * @param str The string to search for.
 * @return True if the exact string is found, false otherwise.
 */
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

/**
 * @brief Reads a file line by line, cleaning each line and returning a vector
 * of non-empty lines.
 *
 * @param path The path to the file.
 * @return A vector of cleaned, non-empty lines.
 */
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

/**
 * @brief Removes curly braces '{' and '}' from a line and cleans it of whitespace.
 *
 * @param line The line to be cleaned.
 * @return The cleaned line without braces.
 */
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

/**
 * @brief Removes the first slash from a path if it exists.
 *
 * @param path The path to modify.
 * @return The path without the leading slash.
 */
std::string delete_first_slash(std::string path)
{
    if (path[0] == '/')
        path.erase(0, 1);
    return path;
}
