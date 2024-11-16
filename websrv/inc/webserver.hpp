/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/15 10:24:29 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/16 23:29:47 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 08:43:27 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/15 00:01:34 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef WEBSERVER_HPP
# define WEBSERVER_HPP
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include "Logger.hpp"
#include "http_enum_codes.hpp"
#include "ws_structs.hpp"
#include "ws_permissions_bitwise.hpp"

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
#include <csignal>

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
#define WS_MAX_RETRIES 5
#define WS_RETRY_DELAY_MICROSECONDS 100000

// TODO: define a path max for WS only, path max is defined at limits.h
# ifndef PATH_MAX
#define PATH_MAX 4096
# endif

// Methods included at http_codes_helper.cpp
std::string method_enum_to_string(int method);
e_methods method_string_to_enum(const std::string& method);
std::string http_status_description(e_http_sts code);
std::map<std::string, std::string> create_mime_types();
std::string get_mime_type(const std::string& path);
bool valid_mime_type(const std::string& path);
std::string replace_template(std::string content, const std::string& key, const std::string& value);
bool black_list_extension(const std::string& path);
bool is_valid_size_t(const std::string& value);
size_t str_to_size_t(const std::string& value);
std::string to_lowercase(const std::string& input);
std::string get_header_value(std::string& haystack, std::string needle, const std::string& sep="\r\n");
std::string trim(const std::string& str, const std::string& chars_to_trim);
bool is_cgi(const std::string& filename);
size_t end_of_header_system(std::string& header);


struct CommandPair {
    const char* command;
    void (*function)(std::vector<std::string>::iterator&, Logger*, LocationConfig&);
};


void print_server_config(const ServerConfig& config, std::string location);
void print_vector_config(const std::vector<ServerConfig> &config, std::string location);
std::string print_bitwise_method(unsigned char method);

std::string html_codes(int code);

// Parse

std::vector<ServerConfig> parse_file(std::string file, Logger* logger);
std::vector<ServerConfig> parse_servers(std::vector<std::string> rawLines, Logger* logger);
LocationConfig parse_location_block(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end, Logger* logger);

// Print

void print_raw_lines(std::vector<std::string> rawLines);
void print_server_config(ServerConfig server);
void print_location_config(LocationConfig location);
void print_servers(std::vector<ServerConfig> servers);


// Utils

std::string get_value(std::string line, const std::string& key);
std::string clean_line(std::string line);
bool find_exact_string(const std::string& line, const std::string& str);
std::vector<std::string> split_default_pages(std::string default_pages);
std::map<int, std::string> split_error_pages(std::string error_pages);
std::vector<std::string> get_raw_lines(std::string file);
std::string delete_brackets_clean(std::string line);
std::string delete_first_slash(std::string path);
std::string get_server_root();
std::vector<std::string>::iterator skip_block(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end);
std::vector<std::string>::iterator find_block_end(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end);
std::string get_location_path(std::string line);
t_mode string_to_error_mode(std::string error_mode);
std::string join_paths(std::string path1, std::string path2);
std::vector<std::string> split_string(std::string str);
unsigned char method_bitwise(std::string parsed);
std::string get_first_word(const std::string& str);

// Verifications

bool check_args(int argc, char **argv);
bool check_root(std::string root);
bool check_client_max_body_size(std::string client_max_body_size);
int check_port(std::string port);
bool check_server_name(std::string server_name);
bool check_error_page(std::string error_page);
bool check_default_page(std::string default_page);
bool check_brackets(std::vector<std::string>::iterator start);
bool check_brackets(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end);
bool check_autoindex(std::string autoindex);
t_allowed_methods string_to_method(std::string method);
bool check_error_mode(std::string error_mode);
bool check_duplicate_servers(std::vector<ServerConfig> servers);
bool check_cgi(std::string cgi);
bool check_obligatory_params(ServerConfig server, Logger* logger);

// Parse Server
void parse_location(std::vector<std::string>::iterator& it, std::vector<std::string>::iterator end, Logger* logger, ServerConfig& server);
void parse_template_error_page(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server);
void parse_port(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server);
void parse_server_name(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server);
void parse_root(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server);
void parse_index(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server);
void parse_client_max_body_size(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server);
void parse_error_page(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server);
void parse_autoindex(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server);
void parse_error_mode(std::vector<std::string>::iterator& it, Logger* logger, ServerConfig& server);

// Parse Location
void parse_location_index(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location);
void parse_location_error_page(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location);
void parse_root(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location);
void parse_autoindex(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location);
void parse_cgi(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location);
void parse_template_error_page(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location);
void parse_accept_only(std::vector<std::string>::iterator& it, Logger* logger, LocationConfig& location);

# endif

