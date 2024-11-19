/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   verifications.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/11/17 14:43:45 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"
#include <filesystem> // Asegúrate de incluir esta biblioteca
#include <sys/stat.h> // Para usar la función stat


/**
 * @brief Checks if the brackets in the given range are balanced.
 *
 * @param start Iterator to the start of the range.
 * @param end Iterator to the end of the range.
 * @return true if brackets are balanced, false otherwise.
 */
bool check_brackets(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end)
{
    int bracketCount = 0;
    for (std::vector<std::string>::iterator it = start; it != end; ++it)
    {
        if (it->find("{") != std::string::npos)
            bracketCount++;
        if (it->find("}") != std::string::npos)
            bracketCount--;
    }
    return bracketCount == 0;
}

/**
 * @brief Checks if a file can be opened.
 *
 * @param filename The name of the file to check.
 * @return true if the file can be opened, false otherwise.
 */
// VERIFY: No estoy seguro de usar esto
bool can_open_file(const char* filename)
{
    std::ifstream file(filename);
    return file.is_open();
}

/**
 * @brief Validates the command line arguments.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return true if arguments are valid, false otherwise.
 */
bool check_args(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Args error. Type --help for instructions." << std::endl;
        return false;
    }

    if (std::string(argv[1]) == "--help")
    {
        std::cout << "Usage: ./webserv <config_file>" << std::endl;
        return false;
    }

    if (argc > 2)
    {
        std::cout << "Error: too many arguments." << std::endl;
        return false;
    }

    if (!can_open_file(argv[1]))
    {
        std::cout << "Error: file \"" << argv[1] << "\" can't be opened." << std::endl;
        return false;
    }
    else
    {
        std::ifstream file(argv[1]);
        if (!file) {
            std::cout << "Error: the file can't be opened." << std::endl;
            return false;
        }
        file.seekg(0, std::ios::end);
        if (file.tellg() == 0) {
            std::cout << "Error: the file is empty." << std::endl;
            return false;
        }
        file.seekg(0, std::ios::beg);
    }

    std::string filename = argv[1];
    if (filename.find_last_of(".") != std::string::npos) {
        if (filename.substr(filename.find_last_of(".") + 1) != "conf") {
            std::cout << "Error: the file must have the .conf extension" << std::endl;
            return false;
        }
    } else {
        std::cout << "Error: the file has no extension." << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Validates a port number.
 *
 * @param port The port number as a string.
 * @return The port number as an integer if valid, -1 otherwise.
 */
int check_port(std::string port)
{
    int port_int = 0;

    if (port.empty()) {
        return -1;
    }
    for (size_t i = 0; i < port.length(); i++)
    {
        if (!isdigit(port[i]))
            return -1;
    }
    port_int = atoi(port.c_str());
    if (port_int <= 1024 || port_int > 65535)
        return -1;
    return port_int;
}

/**
 * @brief Validates a server name.
 *
 * @param server_name The server name to validate.
 * @return true if the server name is valid, false otherwise.
 */
bool check_server_name(std::string server_name)
{
    if (server_name.empty())
        return false;
    if (server_name.length() < 1 || server_name.length() > 253) {
        return false;
    }
    // for (size_t i = 0; i < server_name.length(); i++)
    // {
    //     if (!isalnum(server_name[i]))
    //         return false;
    // }
    return true;
}

/**
 * @brief Checks if an error code is valid.
 *
 * @param code The error code to check.
 * @return true if the error code is valid, false otherwise.
 */
bool is_valid_error_code(int code) {
    return (code >= 400 && code < 600);
}


bool check_error_page(std::string error_page)
{
    if (error_page.empty())
        return false;

    std::istringstream iss(error_page);
    std::string part;
    bool hasErrorCode = false;
    std::map<int, std::string> error_pages;
    int code;

    while (iss >> part) {
        if (isdigit(part[0])) {
            for (size_t i = 0; i < part.length(); i++) {
                if (!isdigit(part[i])) {
                    return false;
                }
                code = atoi(part.c_str());
                if (!is_valid_error_code(code)) {
                    return false;
                }
            }            
            hasErrorCode = true;
        } else if (part[0] == '/') {
            if (!hasErrorCode) {
                return false;
            }
            if (part.find_last_of(".") != std::string::npos) {
                std::string extension = part.substr(part.find_last_of(".") + 1);
                if (extension != "html" && extension != "htm") {
                    return false;
                }
            } else {
                return false;
            }
            return true;
        } else {
            return false;
        }
    }

    return false;
}

/**
 * @brief Checks if a filename is valid.
 *
 * @param filename The filename to check.
 * @return true if the filename
 */
bool is_valid_filename(const std::string& filename) {
    for (size_t i = 0; i < filename.length(); ++i) {
        if (!isalnum(filename[i]) && filename[i] != '.' && filename[i] != '-' && filename[i] != '_') {
            return false;
        }
    }
    return true;
}

/**
 * @brief Checks if the default page is valid.
 *
 * @param default_page The default page to check.
 * @return true if the default page is valid, false otherwise.
 */
bool check_default_page(std::string default_page)
{
    if (default_page.empty())
        return false;
    return true;
}

bool is_directory(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    } else if (info.st_mode & S_IFDIR) {
        return true;
    }
    return false;
}


bool check_root(std::string root)
{
    if (root.empty())
        return false;
    return true;
}

bool check_client_max_body_size(std::string client_max_body_size)   
{
    if (client_max_body_size.empty())
        return false;

    char unit = client_max_body_size[client_max_body_size.size() - 1];
    if (unit != 'M' && unit != 'k')
        return false;

    std::string number_part = client_max_body_size.substr(0, client_max_body_size.size() - 1);
    for (std::string::size_type i = 0; i < number_part.size(); ++i) {
        if (!isdigit(number_part[i]))
            return false;
    }

    return true;
}

bool check_autoindex(std::string autoindex)
{
    if (autoindex == "on")
        return true;
    if (autoindex == "off")
        return true;
    return false;
}

bool check_error_mode(std::string error_mode)
{
    return (error_mode == "literal" || error_mode == "template");
}

bool check_duplicate_servers(std::vector<ServerConfig> servers)
{
    for (size_t i = 0; i < servers.size(); i++)
    {
        for (size_t j = i + 1; j < servers.size(); j++)
        {
            if (servers[i].port == servers[j].port && 
                servers[i].server_name == servers[j].server_name)
                return true;
        }
    }
    return false;
}

bool check_cgi(std::string cgi)
{
    return (cgi == "on" || cgi == "off");
}

bool check_obligatory_params(ServerConfig& server, Logger* logger)
{
    logger->log(LOG_DEBUG, "check_obligatory_params", "Checking obligatory parameters");
    if (server.locations.size() > 1)
    {
        std::map<std::string, LocationConfig>::iterator it1, it2;
        for (it1 = server.locations.begin(); it1 != server.locations.end(); ++it1)
        {
            it2 = it1;
            ++it2;
            for (; it2 != server.locations.end(); ++it2)
            {   // Verifica nuevamente este caso NO OLVIDAR por el .size
                if (it1->second.loc_root == it2->second.loc_root && it1->second.redirections.size() == 0 && it2->second.redirections.size() == 0)
                {
                    logger->log(LOG_ERROR, "check_obligatory_params", 
                        "Locations with the same root: " + it1->second.loc_root);
                    return true;
                }
            }
        }
    }
    if (server.locations.find("/") != server.locations.end() && server.locations["/"].loc_root == "")
        server.locations["/"].loc_root = "";
    for (std::map<std::string, LocationConfig>::iterator it = server.locations.begin(); it != server.locations.end(); ++it)
    {
        if (it->second.loc_default_pages.size() == 0)
        {
            it->second.loc_default_pages = server.default_pages;
            logger->log(LOG_DEBUG, "check_obligatory_params", 
                "Copied default pages to location " + it->first);
        }
    }

    return (server.port == -42 || server.server_root == "" || server.default_pages.size() == 0 || server.locations.size() == 0);
}

