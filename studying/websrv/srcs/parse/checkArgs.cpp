/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   checkArgs.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/22 00:05:49 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"


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

    // Verificar que sean accesibles los ficheros

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
    if (port_int < 1 || port_int > 65535)
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
    for (size_t i = 0; i < server_name.length(); i++)
    {
        if (!isalnum(server_name[i]))
            return false;
    }
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
        if (!isalnum(filename[i]) && filename[i] != '.' && filename[i] != '-') {
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
    std::istringstream iss(default_page);
    std::string page;
    while (iss >> page) {
        if (!is_valid_filename(page)) {
            std::cout << RED << "Error: Página por defecto inválida: " << page << RESET << std::endl;
            return false;
        }
    }
    return true;
}
