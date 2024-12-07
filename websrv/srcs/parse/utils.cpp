/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/11/15 00:02:29 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

/**
 * @brief Cleans a line by removing whitespace and special characters.
 *
 * This function performs several cleaning operations on a string:
 * - Removes leading and trailing whitespace
 * - Removes trailing semicolons
 * - Handles empty lines and comments
 *
 * @param line The input string to be cleaned.
 * @return The cleaned string, or empty string if the input is a comment or empty.
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
 * @brief Extracts a value associated with a specific key from a line.
 *
 * Searches for a key in the given line and extracts the value that follows it.
 * Handles trailing slashes and whitespace in the extracted value.
 *
 * @param line The line containing the key-value pair.
 * @param key The key to search for.
 * @return The extracted value, or empty string if key not found.
 */
std::string get_value(std::string line, const std::string& key) {
    std::string::size_type keyPos = line.find(key);
    if (keyPos != std::string::npos) {
        std::string::size_type valueStart = line.find_first_not_of(" \t", keyPos + key.length());
        if (valueStart != std::string::npos) {
            std::string::size_type valueEnd = line.find_last_not_of(" \t");
            if (valueEnd != std::string::npos && valueEnd >= valueStart) {
                std::string value = line.substr(valueStart, valueEnd - valueStart + 1);
                if (value[value.length() - 1] == '/')
                    value.erase(value.length() - 1);
                return value;
            }
        }
    }
    return "";
}

/**
 * @brief Checks if a string exists as an exact word match at the start of a line.
 *
 * Performs a case-sensitive search for an exact word match, ensuring the string
 * is not part of a larger word.
 *
 * @param line The line to search in.
 * @param str The string to search for.
 * @return true if exact match is found, false otherwise.
 */
bool find_exact_string(const std::string& line, const std::string& str) {
    std::string::size_type end_of_first_word = line.find_first_of(" \t");
    std::string first_word = line.substr(0, end_of_first_word);
    
    size_t pos = first_word.find(str);
    
    if (pos != std::string::npos) {
        if (pos > 0) {
            char prevChar = first_word[pos - 1];
            if (std::isalnum(prevChar) || prevChar == '_') {
                return false;
            }
        }
        
        size_t afterPos = pos + str.length();
        if (afterPos < first_word.length()) {
            char nextChar = first_word[afterPos];
            if (std::isalnum(nextChar) || nextChar == '_') {
                return false;
            }
        }
        return true;
    }
    return false;
}

/**
 * @brief Reads and processes a file line by line.
 *
 * Opens a file, reads each line, cleans it, and stores non-empty lines
 * in a vector.
 *
 * @param path Path to the file to read.
 * @return Vector containing all non-empty, cleaned lines from the file.
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
 * @brief Removes curly braces from a line and cleans whitespace.
 *
 * @param line The line to process.
 * @return The cleaned line with braces removed.
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
 * @brief Removes the leading slash from a path if present.
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

/**
 * @brief Gets the current working directory of the server.
 *
 * @return The current working directory with trailing slash, or empty string on error.
 */
std::string get_server_root()
{
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) != NULL) {
        return std::string(buffer) + "/";
    }
    return "";
}

/**
 * @brief Skips over a block of code between matching brackets.
 *
 * @param start Iterator to the start of the block.
 * @param end Iterator to the end of the range.
 * @return Iterator pointing after the closing bracket of the block.
 */
std::vector<std::string>::iterator skip_block(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end)
{
    int bracketCount = 0;
    for (std::vector<std::string>::iterator it = start; it != end; ++it)
    {
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

/**
 * @brief Finds the end of a block marked by matching brackets.
 *
 * @param start Iterator to the start of the block.
 * @param end Iterator to the end of the range.
 * @return Iterator pointing to the closing bracket.
 */
std::vector<std::string>::iterator find_block_end(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end) {
    int bracketCount = 0;
    for (std::vector<std::string>::iterator it = start; it != end; ++it) {
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
    return end;
}

/**
 * @brief Converts a string method name to its enumerated type.
 *
 * @param method The HTTP method name as string.
 * @return The corresponding t_allowed_methods enum value.
 */
t_allowed_methods string_to_method(std::string method) {
    if (method == "GET")
        return GET;
    if (method == "POST")
        return POST;
    if (method == "DELETE")
        return DELETE;
    return INVALID_METHOD;
}

/**
 * @brief Removes '=' and '~' signs from a line and cleans it.
 *
 * @param line The line to process.
 * @return The cleaned line without specified signs.
 */
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

/**
 * @brief Removes trailing slash from a path if present.
 *
 * @param path The path to modify.
 * @return The path without trailing slash.
 */
std::string delete_last_slash(std::string path) {
    if (path[path.length() - 1] == '/')
        path.erase(path.length() - 1);
    return path;
}

/**
 * @brief Extracts the path from a location directive.
 *
 * Processes a location directive line to extract and clean the path component.
 *
 * @param line The location directive line.
 * @return The cleaned location path.
 */
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

/**
 * @brief Converts an error mode string to its enumerated type.
 *
 * @param error_mode The error mode as string.
 * @return The corresponding t_mode enum value.
 */
t_mode string_to_error_mode(std::string error_mode) {
    if (error_mode == "literal")
        return LITERAL;
    if (error_mode == "template")
        return TEMPLATE;
    return INVALID_ERROR_MODE;
}

/**
 * @brief Joins two paths together, handling slashes appropriately.
 *
 * @param path1 First path component.
 * @param path2 Second path component.
 * @return The combined path.
 */
std::string join_paths(std::string path1, std::string path2) {
    if (path1[path1.length() - 1] == '/')
        path1.erase(path1.length() - 1);
    if (path2[0] == '/')
        path2.erase(0, 1);
    return path1 + "/" + path2;
}

/**
 * @brief Converts HTTP method string to its corresponding bitmask.
 *
 * @param parsed The HTTP method as string.
 * @return The method's bitmask value.
 */
unsigned char method_bitwise(std::string parsed) {
    static std::map<std::string, unsigned char> methods;
    if (methods.empty()) {
        methods.insert(std::make_pair("GET", MASK_METHOD_GET));
        methods.insert(std::make_pair("OPTIONS", MASK_METHOD_OPTIONS));
        methods.insert(std::make_pair("HEAD", MASK_METHOD_HEAD));
        methods.insert(std::make_pair("POST", MASK_METHOD_POST));
        methods.insert(std::make_pair("PUT", MASK_METHOD_PUT));
        methods.insert(std::make_pair("PATCH", MASK_METHOD_PATCH));
        methods.insert(std::make_pair("TRACE", MASK_METHOD_TRACE));
        methods.insert(std::make_pair("DELETE", MASK_METHOD_DELETE));
    }
    
    std::map<std::string, unsigned char>::const_iterator it = methods.find(parsed);
    if (it != methods.end()) {
        return (it->second);
    }
    return (0);
}

/**
 * @brief Extracts the first word from a string.
 *
 * @param str The input string.
 * @return The first word found in the string.
 */
std::string get_first_word(const std::string& str) {
    std::string::size_type pos = str.find_first_of(" \t\n");
    return str.substr(0, pos);
}

/**
 * @brief Validates and converts a status code string to integer.
 *
 * @param status_code The status code as string.
 * @param logger Pointer to the logger instance.
 * @return The validated status code as integer.
 * @throw Logger::fatal_log if status code is invalid.
 */
int get_status_code(std::string status_code, Logger* logger) {
    logger->log(LOG_DEBUG, "get_status_code", "Getting status code");
    if (status_code.find_first_not_of("0123456789") != std::string::npos || status_code.empty() || std::atoi(status_code.c_str()) < 0 || std::atoi(status_code.c_str()) > 599)
        logger->fatal_log("parse_location_block", "Status code " + status_code + " is not valid.");
    return std::atoi(status_code.c_str());
}

/**
 * @brief Processes and validates a redirection URL.
 *
 * @param redirection The redirection directive.
 * @param logger Pointer to the logger instance.
 * @return The processed redirection URL.
 */
std::string get_redirection_url(std::string redirection, Logger* logger) {
    std::string url = get_value(redirection, "redirection");
    logger->log(LOG_DEBUG, "get_redirection_url", "Redirection URL " + url + ".");

    // if (url.find("http://") != std::string::npos || url.find("https://") != std::string::npos || url.find("www.") != std::string::npos)
    // {
    //     logger->log(LOG_DEBUG, "get_redirection_url", "Redirection URL " + url + " is valid.");
    //     return url;
    // }
    // else
    // {
    //     logger->log(LOG_DEBUG, "get_redirection_url", "Redirection URL " + url + " is not valid. Joining with server root.");
    //     return join_paths(get_server_root(), url);
    // }
    return url;
}

/**
 * @brief Splits redirection directives into status code and URL pairs.
 *
 * @param it Iterator to the redirection directive.
 * @param logger Pointer to the logger instance.
 * @return Map of status codes to their corresponding redirection URLs.
 */
std::map<int, std::string> split_redirections(std::vector<std::string>::iterator& it, Logger* logger) {
    std::map<int, std::string> new_redirections;
    std::string redirection_str = get_value(*it, "redirection");
    logger->log(LOG_DEBUG, "split_redirections", "Splitting redirections");
    
    size_t space_pos = redirection_str.find(" ");
    if (space_pos != std::string::npos) {
        std::string status_code_str = redirection_str.substr(0, space_pos);
        std::string url = redirection_str.substr(space_pos + 1);
        
        int status_code = get_status_code(status_code_str, logger);
        std::string redirect_url = url;
        
        if (url.find("http://") != std::string::npos || 
            url.find("https://") != std::string::npos || 
            url.find("www.") != std::string::npos) {
            redirect_url = url;
        } else {
            redirect_url = join_paths(get_server_root(), url);
        }
        
        new_redirections[status_code] = redirect_url;
    }
    
    return new_redirections;
}

/**
 * @brief Compares two paths for equality, handling special cases.
 *
 * @param path1 First path to compare.
 * @param path2 Second path to compare.
 * @return true if paths are equivalent, false otherwise.
 */
bool compare_paths(std::string path1, std::string path2) {
    if (path1 == "/" && path2 != "/")
        return false;

    if (path1[path1.length() - 1] == '/')
        path1.erase(path1.length() - 1);
    if (path2[path2.length() - 1] == '/')
        path2.erase(path2.length() - 1);
    
    size_t pos1 = path1.find_last_of('/');
    size_t pos2 = path2.find_last_of('/');
    
    std::string last_part1 = (pos1 == std::string::npos) ? path1 : path1.substr(pos1 + 1);
    std::string last_part2 = (pos2 == std::string::npos) ? path2 : path2.substr(pos2 + 1);
    return last_part1 == last_part2;
}

/**
 * @brief Converts a size string with units to bytes.
 *
 * Handles size specifications with K/M/G suffixes.
 *
 * @param client_max_body_size Size string with optional unit suffix.
 * @return The size in bytes.
 */
size_t string_to_bytes(std::string client_max_body_size)
{
    size_t bytes = atoi(client_max_body_size.c_str());
    char unit = client_max_body_size[client_max_body_size.size() - 1];
    if (unit == 'm' || unit == 'M')
        bytes *= 1024 * 1024;
    else if (unit == 'k' || unit == 'K')
        bytes *= 1024;
    else if (unit == 'g' || unit == 'G')
        bytes *= 1024 * 1024 * 1024;
    return bytes;
}
