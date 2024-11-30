/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/30 17:45:43 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SocketHandler.hpp"

/**
 * @brief Constructs a new SocketHandler object.
 *
 * This constructor initializes a socket, sets its options, binds it to a specific port, and sets it in listening mode.
 * Additionally, it configures the logger, manages server configurations, and performs initialization for cache instances.
 *
 * @param port The port number on which the server will listen for incoming connections.
 * @param config A reference to the server configuration (ServerConfig) containing server settings.
 * @param logger A pointer to a logger instance (Logger) for logging purposes.
 *
 * @throws Logger::NoLoggerPointer If the provided logger pointer is null.
 * @throws WebServerException If an error occurs while creating the socket, setting socket options, binding, listening, or setting non-blocking mode.
 *
 * @details
 * The constructor follows these steps to establish a socket:
 * 1. Checks if a valid logger pointer is provided, throws if null.
 * 2. Creates a socket using `socket()` function, throwing an exception if it fails.
 * 3. Sets the socket option `SO_REUSEADDR` to reuse local addresses.
 * 4. Binds the socket to the provided port using the `bind()` function.
 * 5. Sets the socket in listening mode to accept incoming connections.
 * 6. Sets the socket to non-blocking mode using `set_nonblocking()`.
 * 7. Logs relevant information at various stages to provide detailed flow insights.
 * 8. Maps CGI extensions (.py, .pl) to handle dynamic requests as part of initialization.
 *
 * @note Throws an exception and properly closes the socket on any failure, preventing resource leaks.
 */
SocketHandler::SocketHandler(int port, ServerConfig& config, const Logger* logger):
		_socket_fd(-1),
        _config(config),
        _log(logger),
		_cache(WebServerCache<CacheEntry>(100)),
		_request_cache(WebServerCache<CacheRequest>(100)) {
	if (_log == NULL) {
		throw Logger::NoLoggerPointer();
	}
	_log->log_info( SH_NAME,
			  "Instance building start.");
	_log->log_debug( SH_NAME,
	          "Creating Sockets.");
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd < 0) {
		throw WebServerException("Error Creating Socket.");
	}
	int opt = 1;
	if (setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		close(_socket_fd);
		throw WebServerException("Error setting socket options.");
	}
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
	if (bind(_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		close(_socket_fd);
		throw WebServerException("Error Linking Socket.");
	}
	_port_str = int_to_string(port);
	_log->log_debug( SH_NAME, "Linking Socket.");
	_log->log_debug( SH_NAME,
			  "Socket to listening mode.");
	if (listen(_socket_fd, SOCKET_BACKLOG_QUEUE) < 0) {
		close_socket();
		throw WebServerException("Error Listening Socket.");
	}
	if (!set_nonblocking(_socket_fd)) {
		close_socket();
		throw WebServerException("Error setting socket as non blocking.");
	}
	_log->log_info( SH_NAME,
					"Server listening. Port: " + int_to_string(port));
	add_host(config);
	_log->log_info( SH_NAME,
	          "Instance built.");
	_log->status(SH_NAME, "Socket Handler Instance is ready.");
}

/**
 * @brief Destroys the SocketHandler object.
 *
 * This destructor cleans up resources by closing the socket and logs the cleanup process.
 *
 * @details
 * The destructor calls `close_socket()` to properly close the socket and release any associated resources.
 * It then logs a debug message indicating that the SocketHandler resources have been cleaned up.
 *
 * @note Ensure that any allocated resources are properly cleaned up to prevent resource leaks.
 */
SocketHandler::~SocketHandler() {
	close_socket();
	_log->log_debug( SH_NAME,
					 "SockedHandler resources clean up.");
}

/**
 * @brief Adds a host to the internal map of hosts if it doesn't already exist.
 *
 * This method takes a reference to a `ServerConfig` object and adds it to the internal map of hosts (`_hosts`) managed by `SocketHandler`.
 * If the host (identified by `server_name` in the `ServerConfig`) does not already exist in `_hosts`, it is added to the map and additional
 * configurations such as redirections and CGI mappings are performed.
 *
 * @param config A reference to the `ServerConfig` object representing the server configuration to be added.
 *
 * The method performs the following steps:
 * 1. Converts the `server_name` from the provided `ServerConfig` to lowercase for consistent key storage.
 * 2. Searches for the `server_name` in the `_hosts` map.
 * 3. If the host is not found (`_hosts.end()`), logs the addition of the new host, adds the configuration to the `_hosts` map, and then
 *    performs additional setup for redirections and CGI script mappings:
 *    - Calls `mapping_redir(config)` to handle redirection setup.
 *    - Calls `mapping_cgi_locations(config, ".py")` and `mapping_cgi_locations(config, ".pl")` to map CGI scripts for Python and Perl respectively.
 *
 * @note This method does not add the host if it already exists in the `_hosts` map.
 *
 */
void SocketHandler::add_host(ServerConfig &config) {
	std::string host_name = to_lowercase(config.server_name);
	std::map<std::string, ServerConfig*>::iterator it =  _hosts.find(host_name);
	if (it == _hosts.end()) {
		_log->status(SH_NAME, "Append host to map.");
		_hosts[host_name] = &config;
		mapping_redir(config);
		mapping_cgi_locations(config, ".py");
		mapping_cgi_locations(config, ".pl");
	}
}

/**
 * @brief Closes the socket associated with the SocketHandler.
 *
 * This method attempts to close the socket if it is still open.
 * It first checks the socket file descriptor using `fcntl()` to see if it is valid and not closed.
 * If the socket is still valid, it proceeds to close it.
 *
 * If an exception occurs while closing the socket, an error is logged with details of the exception.
 *
 * @note Properly closing the socket is important to avoid resource leaks.
 */
void SocketHandler::close_socket() {
	try {
		int flags;
		flags = fcntl(_socket_fd, F_GETFL);
		if (flags != -1 && errno != EBADF) {
			close(_socket_fd);
		}
	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Error closing socket fd.: " << e.what();
		_log->log_error( SH_NAME,
				  detail.str());
	}
}

/**
 * @brief Gets the port number as a string.
 *
 * This method returns the port number to which the socket is bound, represented as a string.
 *
 * @return A string representing the port number.
 */
std::string SocketHandler::get_port() const {
	return (_port_str);
}

/**
 * @brief Accepts a new incoming connection.
 *
 * This method accepts an incoming connection on the socket. If a client connection is accepted successfully, the client socket is set to non-blocking mode.
 * If there is an error while accepting the connection, a warning is logged.
 *
 * @return The file descriptor for the accepted client connection, or -1 if an error occurs.
 */
int SocketHandler::accept_connection() {
	_log->log_debug( SH_NAME,
					 "Accepting Connection.");
	int client_fd = accept(_socket_fd, NULL, NULL);
	if (client_fd < 0) {
		_log->log_warning( SH_NAME,
						   "Error accepting connection.");
	} else {
		set_nonblocking(client_fd);
		_log->log_info( SH_NAME,
						"Connection Accepted.");
	}
	return (client_fd);
}

/**
 * @brief Gets the socket file descriptor.
 *
 * This method returns the file descriptor for the socket associated with the SocketHandler.
 *
 * @return The socket file descriptor.
 */
int SocketHandler::get_socket_fd() const {
	return (_socket_fd);
}

/**
 * @brief Gets the server configuration.
 *
 * This method returns a constant reference to the server configuration associated with the SocketHandler.
 *
 * @return A constant reference to the server configuration (ServerConfig).
 */
ServerConfig& SocketHandler::get_config() const {
	return (_config);
}

ServerConfig* SocketHandler::get_config(const std::string &host) {
	size_t find_host;

	for (std::map<std::string, ServerConfig*>::iterator it = _hosts.begin();
												it != _hosts.end(); it++) {
		find_host = host.find(it->first);
		if (find_host != std::string::npos) {
			return (it->second);
		}
	}
	return (&_config);
}

/**
 * @brief Sets the socket to non-blocking mode.
 *
 * This method sets the provided socket file descriptor to non-blocking mode using `fcntl()`. If there is an error while getting or setting the socket flags, it logs a warning message.
 *
 * @param fd The file descriptor of the socket to be set as non-blocking.
 *
 * @return `true` if the socket was successfully set to non-blocking mode, `false` otherwise.
 */
bool SocketHandler::set_nonblocking(int fd) {
	_log->log_debug( SH_NAME,
			  "Set connection as nonblocking.");
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		_log->log_warning( SH_NAME,
				  "Error getting socket flags.");
		return (false);
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		_log->log_warning( SH_NAME,
				  "Error setting socket as nonblocking.");
		return (false);
	}
	if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
		_log->log_warning( SH_NAME,
		                   "Error setting FD_CLOEXEC.");
		return (false);
	}
	_log->log_info( SH_NAME,
			  "Socket set as nonblocking.");
	return (true);
}

/**
 * @brief Checks if a given path belongs to a specific location.
 *
 * This method iterates over the configured locations and checks if the given path starts with any of the location keys.
 * It finds the longest matching key and compares its root with the provided location root.
 *
 * @param path The path to check.
 * @param loc_root The root of the location to match.
 *
 * @return `true` if the path belongs to the specified location, `false` otherwise.
 */
bool SocketHandler::belongs_to_location(ServerConfig& host, const std::string& path, const std::string& loc_root) {
	std::string saved_key;
	LocationConfig* result = NULL;

	for (std::map<std::string, LocationConfig>::iterator it = host.locations.begin();
	     it != host.locations.end(); ++it) {
		const std::string& key = it->first;
		if (starts_with(path, key)) {
			if (key.length() > saved_key.length()) {
				result = &it->second;
				saved_key = key;
			}
		}
	}
	if (result) {
		return (result->loc_root == loc_root);
	} else {
		return (false);
	}
}

/**
 * @brief Checks if the given file has the specified CGI extension.
 *
 * This method checks whether the provided filename ends with the specified extension, typically used to determine if a file is a CGI script.
 *
 * @param filename The name of the file to check.
 * @param extension The extension to compare with the filename.
 *
 * @return `true` if the filename ends with the specified extension, `false` otherwise.
 */
bool SocketHandler::is_cgi_file(const std::string &filename, const std::string& extension) {
	return (filename.size() >= extension.length()
	        && filename.compare(filename.size() - extension.length(),
        extension.length(), extension) == 0);
}

/**
 * @brief Gets CGI files from a given directory.
 *
 * This method iterates through a directory and finds CGI files with the specified extension.
 * It recursively checks subdirectories and adds valid CGI files to the provided map.
 * If a file is found to belong to the specified location root, it is added to the map of CGI files.
 *
 * @param directory The directory to search for CGI files.
 * @param loc_root The root of the location to match.
 * @param extension The CGI file extension to search for.
 * @param mapped_files A map where the found CGI files will be stored.
 */
void SocketHandler::get_cgi_files(ServerConfig& host, const std::string& directory, const std::string& loc_root,
                                  const std::string& extension, std::map<std::string, t_cgi>& mapped_files) {
	DIR* dir = opendir(directory.c_str());
	if (dir == NULL) {
		_log->log_info( SH_NAME,
		          "No directory was found.");
		return;
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
		if (name == "." || name == "..") {
			continue;
		}

		std::string full_path = directory + "/" + name;
		struct stat info;
		if (stat(full_path.c_str(), &info) != 0) {
			_log->log_warning( SH_NAME,
			          full_path + " : is not accessible.");
			continue ;
		}

		if (S_ISDIR(info.st_mode)) {
			get_cgi_files(host, full_path, loc_root, extension, mapped_files);
		} else if (S_ISREG(info.st_mode) && is_cgi_file(name, extension)) {
			std::string clean_path = full_path.substr(host.server_root.length());
			std::string real_path = clean_path.substr(0, clean_path.find(name));
			size_t pos = clean_path.find(extension);
			clean_path = clean_path.substr(0, pos);
			if (belongs_to_location(host, clean_path, loc_root)) {
				std::ostringstream detail;
				detail << "CGI found path for location root <" << loc_root << ">: ["
					   << clean_path << "] - filename : " << name;
				_log->log_debug( SH_NAME,
				          detail.str());
				mapped_files.insert(std::make_pair(clean_path, s_cgi(real_path, name)));
			}
		}
	}
	closedir(dir);
}

/**
 * @brief Maps CGI locations for the given file extension.
 *
 * This method iterates over all the configured locations in the server and checks if CGI is enabled for that location.
 * For each location that has CGI activated, it searches for CGI files with the specified extension in the location's root directory.
 * If CGI files are found, they are added to the `cgi_locations` map of that location.
 *
 * If any CGI files are found for the specified extension, the `cgi_locations` flag in the server configuration is set to `true`.
 *
 * @param extension The file extension to search for CGI scripts (e.g., ".py" or ".pl").
 */
void SocketHandler::mapping_cgi_locations(ServerConfig& host, const std::string& extension) {
	_log->log_info(SH_NAME,
	               "Mapping CGI whitelist for host: " + extension);
	std::map<std::string, std::string> results;
	for (std::map<std::string, LocationConfig>::iterator it = host.locations.begin(); it != host.locations.end(); it++) {
		if (it->second.cgi_file) {
			_log->log_debug( SH_NAME,
			          "Location with CGI activated, mapping for files.");
			get_cgi_files(host, host.server_root + it->second.loc_root,
			              it->second.loc_root, extension, it->second.cgi_locations);
			if (!it->second.cgi_locations.empty()){
				host.cgi_locations = true;
			} else {
				host.cgi_locations = false;
			}
		}
	}
}

/**
 * @brief Maps redirection configurations for server locations.
 *
 * This method iterates over all locations defined in the server configuration (`_config.locations`).
 * If a location contains a non-empty redirection configuration, it sets the `is_redir` flag to `true` for that location.
 *
 * @details
 * The method follows these steps:
 * 1. Iterates over all location entries in the `_config.locations` map.
 * 2. Checks if the `redirections` attribute for each location is not empty.
 *    - If the location has redirection rules, it sets the `is_redir` flag to `true` to indicate that redirections are configured for that location.
 *
 * This allows the server to efficiently determine which locations have redirection logic configured during request handling.
 */
void SocketHandler::mapping_redir(ServerConfig& host) {
	_log->log_info(SH_NAME,
	               "Mapping Redir locations to host.");
	for (std::map<std::string, struct LocationConfig>::iterator it = host.locations.begin();
		it != host.locations.end(); it++) {
		if (!it->second.redirections.empty()) {
			it->second.is_redir = true;
		}
	}
}


/**
 * @brief Gets the general cache.
 *
 * This method returns a reference to the general cache used by the SocketHandler.
 * The cache stores `CacheEntry` objects for managing web server data.
 *
 * @return A reference to the `WebServerCache` object containing `CacheEntry` elements.
 */
WebServerCache<CacheEntry>& SocketHandler::get_cache() {
	return (_cache);
}

/**
 * @brief Gets the request cache.
 *
 * This method returns a reference to the request cache used by the SocketHandler.
 * The cache stores `CacheRequest` objects for managing requests to the server.
 *
 * @return A reference to the `WebServerCache` object containing `CacheRequest` elements.
 */
WebServerCache<CacheRequest>& SocketHandler::get_request_cache() {
	return (_request_cache);
}
