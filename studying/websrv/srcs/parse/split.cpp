/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   split.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/26 00:09:26 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

/**
 * @brief Splits a string of error pages into a map associating error codes
 * with file paths, replacing 'x' in the path with the last digit of the error code.
 *
 * @param error_pages The string containing error pages.
 * @return A map of error codes to file paths.
 */
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

/**
 * @brief Splits a string of default pages into a vector of strings,
 * separated by spaces.
 *
 * @param default_pages The string containing default pages.
 * @return A vector of default page strings.
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


// std::vector<t_methods> split_allowed_methods(std::string allowed_methods)
// {
//     std::vector<t_methods> methods;
//     std::istringstream iss(allowed_methods);
//     std::string method;
//     while (iss >> method) {
//         if (method == "GET")
//             methods.push_back(GET);
//         else if (method == "POST")
//             methods.push_back(POST);
//         else if (method == "DELETE")
//             methods.push_back(DELETE);
//         else if (method == "PUT")
//             methods.push_back(PUT);
//         else if (method == "HEAD")
//             methods.push_back(HEAD);
//     }
//     return methods;
// }