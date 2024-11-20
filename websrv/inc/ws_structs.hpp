/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ws_structs.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/17 21:15:39 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/20 01:03:29 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WS_STRUCTS_HPP
#define WS_STRUCTS_HPP

typedef enum e_mode {
	TEMPLATE=0,
	LITERAL=1,
	INVALID_ERROR_MODE=-42
} t_mode;

enum e_path_type {
	PATH_REGULAR = 0,
	PATH_QUERY = 1,
	PATH_ENCODED = 2,
	PATH_RELATIVE = 3
};

struct s_request {
	std::string     header;
	std::string     body;
	t_methods      	method;
	std::string     path;
	std::string     path_request;
	e_path_type     path_type;
	std::string     query;
	std::string     normalized_path;
	std::string     path_info;
	size_t          content_length;
	std::string     content_type;
	bool            cgi;
	std::string     script;
	std::string     boundary;
	bool            chunks;
	std::string     range;
	std::string     cookie;
	bool            sanity;
	e_http_sts      status;
	bool            autoindex;
	bool            is_redir;
	std::string     referer;
	s_request() : header(""), body(""), method(0), path(""), path_request(""),
				  path_type(PATH_REGULAR), query(""), normalized_path(""),
				  path_info(""), content_length(0), content_type(""),
				  cgi(false), script(""), boundary(""), chunks(false),
				  range(""), cookie(""), sanity(true),
				  status(HTTP_MAX_STATUS), autoindex(false), is_redir(false), referer("") {};
};

typedef struct s_cgi {
	std::string	cgi_path;
	std::string script;
	s_cgi(std::string cp, std::string s): cgi_path(cp), script(s) {};
} t_cgi;


typedef enum e_allowed_methods {
	GET=0,
	POST=1,
	DELETE=2,
	PUT=3,
	INVALID_METHOD=-42
} t_allowed_methods;

std::string int_to_string(int number);
struct LocationConfig {
	std::string                         loc_root;
	std::vector<std::string>            loc_default_pages;
	t_mode                              loc_error_mode; // (?)
	std::map<int, std::string>          loc_error_pages;
	unsigned char                       loc_allowed_methods;
	bool                                autoindex;
	bool                                cgi_file;
	std::map<std::string, t_cgi>		cgi_locations;
	std::map<int, std::string>			redirections;
	bool								is_root;
	bool                                is_redir;
	std::string                         path_root;
	LocationConfig(): loc_error_mode(LITERAL), autoindex(false), cgi_file(false), is_root(false), is_redir(false)
	                  {loc_allowed_methods = 0; loc_default_pages = std::vector<std::string>();};
	LocationConfig(std::string r, std::vector<std::string>& dp, t_mode em, std::map<int, std::string>& ep) :
			loc_root(r), loc_default_pages(dp), loc_error_mode(em), loc_error_pages(ep), loc_allowed_methods(0), cgi_file(true){};

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
	bool										  cgi_locations;  // set after mappig, to avoid config vs files errors
//	------>>> General config, apply to all servers. Here to make it faster at exec
	std::string ws_root;
	std::string ws_errors_root; // Es un root de defecto para las paginas de error (?)
	t_mode      ws_error_mode; // (?)
	ServerConfig(): port(-42) {};
};

struct CacheEntry {
	std::string url;
	std::string content;

	CacheEntry(const std::string &u, const std::string &c)
			: url(u), content(c) {};
	CacheEntry(): url(""), content("") {};
};

struct CacheRequest {
	std::string             url;
	const LocationConfig*   location;
	std::string             normalized_path;

	CacheRequest(const std::string& u, const LocationConfig* loc,
				 const std::string& np): url(u), location(loc), normalized_path(np) {};
	CacheRequest(): url(""), location(NULL), normalized_path("") {};
};

#endif
