/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/14 13:50:38 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef WEBSERVER_HPP
# define WEBSERVER_HPP
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include "http_enum_codes.hpp"
#define NO_LOCATION -1

// Methods included at http_codes_helper.cpp
e_methods method_string_to_enum(const std::string& method);
std::string http_status_description(e_http_sts code);

typedef enum e_mode {
	TEMPLATE=0,
	LITERAL=1
} t_mode;

typedef enum e_access {
	ACCESS_BAD_REQUEST = 0,
	ACCESS_FORBIDDEN = 1,
	ACCESS_READ = 2,
	ACCESS_WRITE = 3
} t_access;

std::string int_to_string(int number);
struct LocationConfig {
	std::string                 loc_root;
	e_access                    loc_access;
	std::vector<std::string>    loc_default_pages;
	t_mode                      loc_error_mode;
	std::map<int, std::string>  loc_error_pages;
	LocationConfig(std::string r, e_access x, std::vector<std::string>& dp, t_mode em, std::map<int, std::string>& ep) :
			loc_root(r), loc_access(x), loc_default_pages(dp), loc_error_mode(em), loc_error_pages(ep) {};
};

struct ServerConfig {
	int port;
	std::string                                   server_name;
	std::string                                   server_root;
	t_mode                                        error_mode;
	std::map<int, std::string>                    error_pages;
	std::map<std::string, LocationConfig>         locations;
	std::vector<std::string>                      default_pages;
//	------>>> General config, apply to all servers. Here to make it faster at exec
	std::string ws_root;
	std::string ws_errors_root;
	t_mode      ws_error_mode;
};
void print_server_config(const ServerConfig& config, std::string location);
void print_vector_config(const std::vector<ServerConfig> &config, std::string location);
bool is_file(std::string ruta);
bool is_dir(std::string ruta);
std::string html_codes(int code);
bool starts_with(const std::string& str, const std::string& prefix);
//void print_server_config(const ServerConfig& config) {
//	std::cout << "Server Configuration:" << std::endl;
//	std::cout << "  Port: " << config.port << std::endl;
//	std::cout << "  Server Name: " << config.server_name << std::endl;
//	std::cout << "  Document Root: " << config.server_root << std::endl;
//
//	// Imprimir las páginas de error personalizadas
//	std::cout << "  Error Pages: " << std::endl;
//	for (std::map<int, std::string>::const_iterator it = config.error_pages.begin(); it != config.error_pages.end(); ++it) {
//		std::cout << "    Error " << it->first << ": " << it->second << std::endl;
//	}
//
//	// Imprimir las páginas por defecto
//	std::cout << "  Default Pages: " << std::endl;
//	for (std::vector<std::string>::const_iterator it = config.default_pages.begin(); it != config.default_pages.end(); ++it) {
//		std::cout << "    " << *it << std::endl;
//	}
//
//	// Imprimir las rutas configuradas en 'locations'
//	std::cout << "  Locations: " << std::endl;
//	for (std::map<std::string, std::string>::const_iterator it = config.locations.begin(); it != config.locations.end(); ++it) {
//		std::cout << "    Path: " << it->first << ", Directory: " << it->second << std::endl;
//	}
//
//	std::cout << std::endl;
//}
//
//void print_vector_config(std::vector<ServerConfig> &config)
//{
//	for (size_t i = 0; i < config.size(); i++)
//		print_server_config(config[i]);
//}

# endif
