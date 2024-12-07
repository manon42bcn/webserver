/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   split.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/11/15 00:02:22 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

/**
 * @brief Splits a string of error pages into a map associating error codes with file paths.
 *
 * This function parses a string containing error page definitions and creates a mapping between
 * HTTP error codes and their corresponding error page file paths.
 *
 * @param error_pages String containing space-separated error codes followed by a file path.
 * @return std::map<int, std::string> Map of error codes to their corresponding file paths.
 */
std::map<int, std::string> split_error_pages(std::string error_pages)
{
    std::map<int, std::string> error_pages_map;
    std::istringstream iss(error_pages);
    std::string token;
    std::vector<int> error_codes;
    std::string path;

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
            path = token;
            break;
        }
    }

    for (std::vector<int>::iterator it = error_codes.begin(); it != error_codes.end(); ++it)
        error_pages_map[*it] = path;
    return error_pages_map;
}

/**
 * @brief Splits a string of default pages into a vector of strings.
 *
 * Parses a space-separated string of default page paths into individual components.
 *
 * @param default_pages String containing space-separated default page paths.
 * @return std::vector<std::string> Vector containing individual default page paths.
 */
std::vector<std::string> split_default_pages(std::string default_pages)
{
    std::vector<std::string> default_pages_vector;
    std::istringstream iss(default_pages);
    std::string page;
    while (iss >> page)
        default_pages_vector.push_back(page);
    return default_pages_vector;
}

/**
 * @brief Splits a string into a vector of substrings based on whitespace.
 *
 * @param str Input string to be split.
 * @return std::vector<std::string> Vector containing individual tokens.
 */
std::vector<std::string> split_string(std::string str) {
    std::vector<std::string> result;
    std::istringstream iss(str);
    std::string token;
    while (iss >> token)
        result.push_back(token);
    return result;
}
