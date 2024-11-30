/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ws_structs.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/17 21:15:39 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/30 17:19:46 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WS_STRUCTS_HPP
#define WS_STRUCTS_HPP
#include "WebserverCache.hpp"

/**
 * @brief Represents different operational modes for processing.
 *
 * This enumeration defines the available modes of operation and their associated constants.
 */
typedef enum e_mode {
	TEMPLATE=0,
	LITERAL=1,
	INVALID_ERROR_MODE=-42
} t_mode;

/**
 * @brief Represents the type of a file path in an HTTP request.
 *
 * This enumeration categorizes file paths into different types based on their
 * nature and usage within the request.
 */
enum e_path_type {
	PATH_REGULAR = 0,
	PATH_QUERY = 1,
	PATH_ENCODED = 2,
	PATH_RELATIVE = 3
};

/**
 * @brief Represents metadata for a CGI (Common Gateway Interface) request.
 *
 * This structure contains information about the path to the CGI executable
 * and the script being executed.
 */
typedef struct s_cgi {
	std::string	cgi_path;
	std::string script;
	s_cgi(std::string cp, std::string s): cgi_path(cp), script(s) {};
} t_cgi;

/**
 * @brief Represents the allowed HTTP methods for a location or server.
 *
 * This enumeration defines the HTTP methods that can be explicitly allowed
 * in the configuration of a server or a specific location. It also includes
 * a value to represent an invalid or unrecognized method.
 */
typedef enum e_allowed_methods {
	GET=0,
	POST=1,
	DELETE=2,
	PUT=3,
	INVALID_METHOD=-42
} t_allowed_methods;

/**
 * @brief Represents the configuration for a specific location within a server.
 *
 * This structure holds all configuration parameters specific to a location
 * (e.g., a particular path or directory) within a server. It includes settings
 * for default pages, error handling, CGI configurations, redirections, and
 * allowed methods.
 */
struct LocationConfig {
	std::string                         loc_root;
	std::vector<std::string>            loc_default_pages;
	t_mode                              loc_error_mode;
	std::map<int, std::string>          loc_error_pages;
	unsigned char                       loc_allowed_methods;
	bool                                autoindex;
	bool                                cgi_file;
	std::map<std::string, t_cgi>		cgi_locations;
	std::map<int, std::string>			redirections;
	bool								is_root;
	bool                                is_redir;
	std::string                         path_root;
	LocationConfig():
			loc_root(),
			loc_default_pages(),
			loc_error_mode(LITERAL),
			loc_error_pages(),
			loc_allowed_methods(0),
			autoindex(false),
			cgi_file(false),
			cgi_locations(),
			redirections(),
			is_root(false),
			is_redir(false),
			path_root() {
		loc_root.clear();
		loc_default_pages = std::vector<std::string>();
		loc_error_pages = std::map<int, std::string>();
		cgi_locations = std::map<std::string, t_cgi>();
		redirections = std::map<int, std::string>();
	};

	LocationConfig(std::string r,
				   std::vector<std::string>& dp,
				   t_mode em,
				   std::map<int, std::string>& ep) :
			loc_root(r),
			loc_default_pages(dp),
			loc_error_mode(em),
			loc_error_pages(ep),
			loc_allowed_methods(0),
			autoindex(false),
			cgi_file(false),
			cgi_locations(),
			redirections(),
			is_root(false),
			is_redir(false),
			path_root() {
		cgi_locations = std::map<std::string, t_cgi>();
		redirections = std::map<int, std::string>();
	}
};

struct CacheRequest;

struct ServerConfig {
	int                                           port;
	std::string                                   server_name;
	std::string                                   server_root;
	t_mode                                        error_mode;
	std::map<int, std::string>                    error_pages;
	std::map<std::string, struct LocationConfig>  locations;
	std::vector<std::string>                      default_pages;
	size_t                                        client_max_body_size;
	bool                                          autoindex;
	std::string                                   template_error_page;
	bool										  cgi_locations;
	//	------>>> General config, apply to all servers. Here to make it faster at exec
	std::string ws_root;
	std::string ws_errors_root;
	t_mode      ws_error_mode;
	WebServerCache<CacheRequest>                  request_cache;

	ServerConfig()
			: port(-42),
			  server_name(),
			  server_root(),
			  error_mode(),
			  error_pages(),
			  locations(),
			  default_pages(),
			  client_max_body_size(0),
			  autoindex(false),
			  template_error_page(),
			  cgi_locations(false),
			  ws_root(),
			  ws_errors_root(),
			  ws_error_mode(),
			  request_cache(WebServerCache<CacheRequest>(100)) {
		error_pages.clear();
		locations.clear();
		default_pages.clear();
		server_name.clear();
		server_root.clear();
		template_error_page.clear();
		ws_root.clear();
		ws_errors_root.clear();
	}
};

/**
 * @brief Represents an HTTP request and its associated metadata.
 *
 * This structure holds all relevant data extracted from an HTTP request,
 * including headers, body, method, path, and other attributes required
 * for processing the request.
 */
struct s_request {
	std::string             header;
	std::string             body;
	std::string             host;
	t_methods               method;
	std::string             method_str;
	std::string             path;
	std::string             path_request;
	e_path_type             path_type;
	std::string             normalized_path;
	std::string             path_info;
	std::string             query;
	std::string             referer;
	size_t                  content_length;
	std::string             content_type;
	std::string             cookie;
	std::string             script;
	std::string             boundary;
	std::string             range;
	bool                    chunks;
	int                     factory;
	bool                    is_cached;
	bool                    autoindex;
	bool                    is_redir;
	bool                    cgi;
	bool                    sanity;
	e_http_sts              status;
	const LocationConfig*   location;
	ServerConfig*           host_config;
	bool                    request_ready;

	s_request():
			header(),
			body(),
			host(),
			method(0),
			method_str(),
			path(),
			path_request(),
			path_type(PATH_REGULAR),
			normalized_path(),
			path_info(),
			query(),
			referer(),
			content_length(0),
			content_type(),
			cookie(),
			script(),
			boundary(),
			range(),
			chunks(false),
			factory(0),
			is_cached(false),
			autoindex(false),
			is_redir(false),
			cgi(false),
			sanity(true),
			status(HTTP_I_AM_A_TEAPOT),
			location(NULL),
			host_config(NULL),
			request_ready(false) {}

	void clear_request () {
		header.clear();
		body.clear();
		host.clear();
		method = 0;
		method_str.clear();
		path.clear();
		path_request.clear();
		path_type = PATH_REGULAR;
		normalized_path.clear();
		path_info.clear();
		query.clear();
		referer.clear();
		content_length = 0;
		content_type.clear();
		cookie.clear();
		script.clear();
		boundary.clear();
		range.clear();
		chunks = false;
		factory = 0;
		is_cached = false;
		autoindex = false;
		is_redir = false;
		cgi = false;
		sanity = true;
		status = HTTP_I_AM_A_TEAPOT;
		location = NULL;
		host_config = NULL;
		request_ready = false;
	}
};

/**
 * @brief Represents an entry in the server's cache.
 *
 * This structure stores a URL and its corresponding content, allowing the
 * server to quickly respond to requests for cached resources.
 */
struct CacheEntry {
	std::string			url;
	std::string			content;

	CacheEntry(const std::string &u, const std::string &c):
	    url(u),
	    content(c) {};
	CacheEntry(): url(), content(){
		url.clear();
		content.clear();
	};
};

/**
 * @brief Represents a request to retrieve a cached resource.
 *
 * This structure stores metadata about a cache request, including the URL,
 * server configuration, location configuration, and normalized path.
 */
struct CacheRequest {
	std::string             url;
	const ServerConfig*     host;
	const LocationConfig*   location;
	std::string             normalized_path;

	CacheRequest(const std::string& u,
				 const ServerConfig* h,
				 const LocationConfig* loc,
				 const std::string& np):
			 url(u),
			 host(h),
			 location(loc),
			 normalized_path(np) {};

	CacheRequest():
			 url(),
			 host(NULL),
			 location(NULL),
			 normalized_path() {
		url.clear();
		normalized_path.clear();
	};
};

#endif
