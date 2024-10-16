/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   checkArgs.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/16 22:28:25 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

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

bool can_open_file(const char* filename)
{
    std::ifstream file(filename);
    return file.is_open();
}

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

bool is_valid_filename(const std::string& filename) {
    for (size_t i = 0; i < filename.length(); ++i) {
        if (!isalnum(filename[i]) && filename[i] != '.' && filename[i] != '-') {
            return false;
        }
    }

    return true;
}

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
