/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/26 20:43:05 by vaguilar         ###   ########.fr       */
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
    if (line.find_first_not_of(" \t\n") == std::string::npos)
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
                // TESTEANDO: si termina en / que se lo quite
                std::string value = line.substr(valueStart, valueEnd - valueStart + 1);
                if (value[value.length() - 1] == '/')
                    value.erase(value.length() - 1);
                // std::cout << RED << "RETORNARE:     " << line.substr(valueStart, valueEnd - valueStart + 1) << RESET << std::endl;
                return value;
            }
        }
    }
    return "";
}

// Realmente no busca el string exacto
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

std::string get_server_root()
{
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) != NULL) {
        return std::string(buffer) + "/";
    }
    return "";
}

std::vector<std::string>::iterator skip_block(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end)
{
    int bracketCount = 0;
    for (std::vector<std::string>::iterator it = start; it != end; ++it)
    {
        // std::cout << GRAY << "Skip block: " << *it << RESET << std::endl;
        if (it->find("{") != std::string::npos)
        {
            bracketCount++;
        }
        if (it->find("}") != std::string::npos)
        {
            bracketCount--;
            if (bracketCount == 0)
            {
                return ++it;
            }
        }
    }
    return end;
}


std::vector<std::string>::iterator find_block_end(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end) {
    int bracketCount = 0;
    for (std::vector<std::string>::iterator it = start; it != end; ++it) {
        // std::cout << GRAY << "Find block end: " << *it << RESET << std::endl;
        if (it->find("{") != std::string::npos) {
            bracketCount++;
        }
        if (it->find("}") != std::string::npos) {
            bracketCount--;
            if (bracketCount == 0) {
                return it;
            }
        }
    }
    // Si no encuentra el final, devuelve el final y da mensaje de error y retorna valor invalido
    std::cout << RED << "No se encontro el final del bloque" << RESET << std::endl;
    return end;
}

t_allowed_methods string_to_method(std::string method) {
    if (method == "GET")
        return GET;
    if (method == "POST")
        return POST;
    if (method == "DELETE")
        return DELETE;
    return INVALID_METHOD;
}


// Elimino los signos '=' y '~' y limpia la linea
std::string delete_signs(std::string line) {
    std::string result;
    for (std::string::iterator it = line.begin(); it != line.end(); ++it) {
        if (*it != '=' && *it != '~') {
            result += *it;
        }
    }
    result = clean_line(result);
    return result;
}

std::string delete_last_slash(std::string path) {
    if (path[path.length() - 1] == '/')
        path.erase(path.length() - 1);
    return path;
}

std::string get_location_path(std::string line) {
    std::string::size_type start_pos = line.find("location");
    std::string location_path;
    if (start_pos != std::string::npos)
    {
        start_pos += 8;
        std::string::size_type end_pos = line.find_first_of("{", start_pos);
        if (end_pos != std::string::npos)
            location_path = line.substr(start_pos, end_pos - start_pos);
        else
            location_path = line.substr(start_pos);
        location_path = delete_first_slash(location_path);
    }
    location_path = delete_signs(location_path);
    if (location_path != "/" && location_path[location_path.length() - 1] == '/')
        location_path.erase(location_path.length() - 1);
    return location_path;
}

t_mode string_to_error_mode(std::string error_mode) {
    if (error_mode == "literal")
        return LITERAL;
    if (error_mode == "template")
        return TEMPLATE;
    return INVALID_ERROR_MODE;
}

std::string join_paths(std::string path1, std::string path2) {
    if (path1[path1.length() - 1] == '/')
        path1.erase(path1.length() - 1);
    if (path2[0] == '/')
        path2.erase(0, 1);
    return path1 + "/" + path2;
}

