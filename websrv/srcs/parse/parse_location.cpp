/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_location.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/11/14 22:29:41 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "webserver.hpp"

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
    location.path_root = root;
    if (check_root(root))
        location.loc_root = get_value(*it, "root");
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

void parse_cgi(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location) {
    if (check_cgi(get_value(*it, "cgi")))
    {
        if (get_value(*it, "cgi") == "on")
            location.cgi_file = true;
        else if (get_value(*it, "cgi") == "off")
            location.cgi_file = false;
    }
    else
        logger->fatal_log("parse_location_block", "CGI " + get_value(*it, "cgi") + " is not valid.");   
}

void parse_template_error_page(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location) {
    logger->log(LOG_DEBUG, "parse_server_block", "Parsing template error page");

    if (get_value(*it, "error_mode") == "template")
        location.loc_error_mode = TEMPLATE;
    else if (get_value(*it, "error_mode") == "literal")
        location.loc_error_mode = LITERAL;
    else
        logger->fatal_log("parse_location_block", "Error mode " + get_value(*it, "error_mode") + " is not valid.");
}

void parse_accept_only(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location) {
    logger->log(LOG_DEBUG, "parse_accept_only", "Parsing accept only block");

    location.loc_allowed_methods = 0;

    std::string accept_only = get_header_value(*it, "accept_only", ";");
    std::vector<std::string> accept_only_methods = split_string(accept_only);
    
    for (std::vector<std::string>::iterator it = accept_only_methods.begin(); 
         it != accept_only_methods.end(); ++it) {
        location.loc_allowed_methods |= method_bitwise(*it);
    }
}


void parse_redirection(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location) {
    logger->log(LOG_DEBUG, "parse_redirection", "Parsing redirection block");
    logger->log(LOG_DEBUG, "parse_redirection", "Splitting redirections, it value is " + *it);
    std::map<int, std::string> new_redirections = split_redirections(it, logger);
    location.redirections.insert(new_redirections.begin(), new_redirections.end());
}


