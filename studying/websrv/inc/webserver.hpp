/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/28 19:08:18 by vaguilar         ###   ########.fr       */
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
#define NO_LOCATION -1

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

#define PATH_MAX 4096

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
std::string get_header_value(std::string& haystack, std::string needle, std::string sep);
std::string trim(const std::string& str, const std::string& chars_to_trim);

typedef enum e_mode {
	TEMPLATE=0,
	LITERAL=1,
	INVALID_ERROR_MODE=-42
} t_mode;

typedef enum e_access {
	ACCESS_BAD_REQUEST = 0,
	ACCESS_FORBIDDEN = 1,
	ACCESS_READ = 2,
	ACCESS_WRITE = 3,
	ACCESS_DELETE = 4
} t_access;

enum e_path_type {
	PATH_REGULAR = 0,
	PATH_QUERY = 1,
	PATH_ENCODED = 2
};

struct s_request {
	std::string& body;
	e_methods&   method;
	std::string& path;
	std::string& normalized_path;
	e_access&    access;
	bool&        sanity;
	e_http_sts&  status;
	size_t&      content_length;
	std::string& content_type;
	std::string& boundary;
	e_path_type& path_type;
	std::string& query;

	s_request(std::string& b, e_methods& m, std::string& p,
	          std::string& np, e_access& a, bool& s, e_http_sts& sts,
	          size_t& cl, std::string& ct, std::string& bd,
			  e_path_type& pt, std::string& qy):
			  body(b), method(m), path(p), normalized_path(np),
              access(a), sanity(s), status(sts),
			  content_length(cl), content_type(ct), boundary(bd),
			  path_type(pt), query(qy){};
};

struct s_path {
	e_http_sts  code;
	bool        found;
	std::string path;
	s_path(e_http_sts c, bool f, const std::string p) : code(c), found(f), path(p) {}
};

struct s_content {
	bool        status;
	std::string content;
	s_content(bool s, std::string c): status(s), content(c){};
};

typedef enum e_allowed_methods {
	GET=0,
	POST=1,
	DELETE=2,
	PUT=3,
	// Invalid method is necessary (?)
	INVALID_METHOD=-42
} t_allowed_methods;

std::string int_to_string(int number);
struct LocationConfig {
	std::string                 loc_root;
	e_access                    loc_access; // (?)
	std::vector<std::string>    loc_default_pages;
	t_mode                      loc_error_mode; // (?)
	std::map<int, std::string>  loc_error_pages;
	std::vector<t_allowed_methods>    loc_allowed_methods;
	bool                        autoindex;

	LocationConfig() {};
	LocationConfig(std::string r, e_access x, std::vector<std::string>& dp, t_mode em, std::map<int, std::string>& ep) :
	loc_root(r), loc_access(x), loc_default_pages(dp), loc_error_mode(em), loc_error_pages(ep) {};
};

struct ServerConfig {
	int                                           port;
	std::string                                   server_name;
	std::string                                   server_root;
	t_mode                                        error_mode; // (?)
	std::map<int, std::string>                    error_pages;
	std::map<std::string, struct LocationConfig>  locations;
	std::vector<std::string>                      default_pages;
    std::string                                   client_max_body_size;
    bool                                          autoindex;
	std::string                                   template_error_page;
//	------>>> General config, apply to all servers. Here to make it faster at exec
	std::string ws_root;
	std::string ws_errors_root; // Es un root de defecto para las paginas de error (?)
	t_mode      ws_error_mode; // (?)
};

void print_server_config(const ServerConfig& config, std::string location);
void print_vector_config(const std::vector<ServerConfig> &config, std::string location);
bool is_file(std::string ruta);
bool is_dir(std::string ruta);
std::string html_codes(int code);
bool starts_with(const std::string& str, const std::string& prefix);

// Parse

std::vector<ServerConfig> parse_file(std::string file, Logger* logger);
std::vector<ServerConfig> parse_servers(std::vector<std::string> rawLines, Logger* logger);
LocationConfig parse_location_block(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end, Logger* logger);
std::vector<t_allowed_methods> parse_limit_except(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end, Logger* logger);

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


# endif

