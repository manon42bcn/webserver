/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 18:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/16 22:51:30 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERVER_H
# define WEBSERVER_H

#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <string>
#include <sstream> 


#define DEL_LINE		"\033[2K"
#define ITALIC			"\033[3m"
#define BOLD			"\033[1m"
#define RESET			"\033[0;39m"
#define GRAY			"\033[0;90m"
#define RED				"\033[0;91m"
#define GREEN			"\033[0;92m"
#define YELLOW			"\033[0;93m"
#define BLUE			"\033[0;94m"
#define MAGENTA			"\033[0;95m"
#define CYAN			"\033[0;96m"
#define WHITE			"\033[0;97m"
#define BLACK			"\033[0;99m"
#define ORANGE			"\033[38;5;209m"
#define BROWN			"\033[38;2;184;143;29m"
#define DARK_GRAY		"\033[38;5;234m"
#define MID_GRAY		"\033[38;5;245m"
#define DARK_GREEN		"\033[38;2;75;179;82m"
#define DARK_YELLOW		"\033[38;5;143m"

typedef enum e_mode {
	TEMPLATE=0,
	LITERAL=1
} t_mode;
typedef enum e_access {
	ACCESS_FORBIDDEN=0,
	ACCESS_READ=1,
	ACCESS_WRITE=2
} t_access;

typedef enum e_methods {
	GET=0,
	POST=1,
	DELETE=2,
	PUT=3,
	HEAD=4,
	OPTIONS=5,
	TRACE=6,
	CONNECT=7
} t_methods;

std::string int_to_string(int number);
struct LocationConfig {
	std::string                 loc_root;
	e_access                    loc_access;
	std::vector<std::string>    loc_default_pages;
	t_mode                      loc_error_mode;
	std::map<int, std::string>  loc_error_pages;
	std::vector<t_methods>    loc_allowed_methods;
	// LocationConfig(std::string r, e_access x, std::vector<std::string>& dp, t_mode em, std::map<int, std::string>& ep) :
	// loc_root(r), loc_access(x), loc_default_pages(dp), loc_error_mode(em), loc_error_pages(ep) {};
};

struct ServerConfig {
	int                                           port;
	std::string                                   server_name;
	std::string                                   server_root;
	t_mode                                        error_mode; // Aun falta
	std::map<int, std::string>                    error_pages;
	std::map<std::string, struct LocationConfig>  locations;
	std::vector<std::string>                      default_pages;
    std::string                                   client_max_body_size;
    bool                                          autoindex;
//	------>>> General config, apply to all servers. Here to make it faster at exec
	std::string ws_root;	// Es el path del ejecutable (?)
	std::string ws_errors_root; // Es un root de defecto para las paginas de error (?)
	t_mode      ws_error_mode;
};

void parse_file(std::string file);
bool check_args(int argc, char **argv);
std::string get_value(std::string line, const std::string& key);
std::string clean_line(std::string line);
int check_port(std::string port);
bool check_server_name(std::string server_name);
bool check_error_page(std::string error_page);
bool check_default_page(std::string default_page);
void print_raw_lines(std::vector<std::string> rawLines);
void print_server_config(ServerConfig server);
bool find_exact_string(const std::string& line, const std::string& str);
std::vector<std::string> split_default_pages(std::string default_pages);
std::map<int, std::string> split_error_pages(std::string error_pages);
bool check_brackets(std::vector<std::string>::iterator start);
std::vector<std::string> get_raw_lines(std::string file);
std::vector<ServerConfig> parse_servers(std::vector<std::string> rawLines);
bool check_brackets(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end);
std::string delete_brackets_clean(std::string line);

#endif
