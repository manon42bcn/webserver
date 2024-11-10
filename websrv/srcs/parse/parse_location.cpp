/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_location.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/11/05 21:36:29 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "webserver.hpp"

std::vector<t_allowed_methods> parse_limit_except(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end, Logger* logger) {
    std::vector<t_allowed_methods> all_methods;
    std::vector<t_allowed_methods> methods;

    std::vector<std::string>::iterator start_copy = start;

    for (int i = 0; i < 4; i++)
        all_methods.push_back(static_cast<t_allowed_methods>(i));

    logger->log(LOG_DEBUG, "parse_limit_except", "Parsing limit except block");

    std::string methods_line = start_copy->substr(12);
    std::istringstream iss(methods_line);
    std::vector<std::string> method_tokens;
    std::string token;
    while (iss >> token) {
        method_tokens.push_back(token);
    }
    for (std::vector<std::string>::iterator it = method_tokens.begin(); it != method_tokens.end(); it++) {
        methods.push_back(string_to_method(*it));
    }
    start_copy++;
    for (std::vector<std::string>::iterator it = start_copy; it != end; it++) {
        if (find_exact_string(*it, "deny"))
        {
            for (std::vector<t_allowed_methods>::iterator mit = methods.begin(); mit != methods.end(); mit++) {
                for (std::vector<t_allowed_methods>::iterator ait = all_methods.begin(); ait != all_methods.end(); ait++) {
                    if (*mit == *ait) {
                        all_methods.erase(ait);
                    }
                }
            }
        }
    }

    return all_methods;
}

void parse_location_index(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location) {
    if (check_default_page(get_value(*it, "index")))
        location.loc_default_pages = split_default_pages(get_value(*it, "index"));
    else
        logger->fatal_log("parse_location_block", "Default page " + get_value(*it, "index") + " is not valid.");
}

void parse_location_error_page(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location) {
    std::string error_page = get_value(*it, "error_page");
    if (check_error_page(error_page))
    {
        std::map<int, std::string> new_error_pages = split_error_pages(error_page);
        location.loc_error_pages.insert(new_error_pages.begin(), new_error_pages.end());
    }
    else
        logger->fatal_log("parse_location_block", "Error page " + error_page + " is not valid.");
}


void parse_root(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location) {
    std::string root = get_value(*it, "root");
    if (check_root(root))
        location.loc_root = join_paths(get_server_root(), get_value(*it, "root"));
    else
        logger->fatal_log("parse_location_block", "Root " + root + " is not valid.");
}

void parse_autoindex(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location) {
    if (check_autoindex(get_value(*it, "autoindex")))
    {
        if (get_value(*it, "autoindex") == "on")
            location.autoindex = true;
        else if (get_value(*it, "autoindex") == "off")
            location.autoindex = false;
    }
    else
        logger->fatal_log("parse_location_block", "Autoindex " + get_value(*it, "autoindex") + " is not valid.");
}

// No usado
void parse_limit_except(std::vector<std::string>::iterator& it, std::vector<std::string>::iterator end, Logger* logger, LocationConfig& location) {
    location.loc_allowed_methods = parse_limit_except(it, end, logger);
}

void parse_cgi(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location) {
    if (check_cgi(get_value(*it, "cgi")))
    {
        if (get_value(*it, "cgi") == "on")
            location.cgi = true;
        else if (get_value(*it, "cgi") == "off")
            location.cgi = false;
    }
    else
        logger->fatal_log("parse_location_block", "CGI " + get_value(*it, "cgi") + " is not valid.");   
}
