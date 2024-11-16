# SocketHandler Class

## Overview
The `SocketHandler` class is a key component for managing socket connections within a web server. It handles the creation, configuration, and management of server sockets, including binding to ports, setting socket options, and managing incoming connections. Additionally, `SocketHandler` provides methods for working with CGI scripts, logging activities, and managing server caches.

## Features
- Socket creation, configuration, and management.
- Binding to specified server ports and listening for incoming connections.
- Setting sockets to non-blocking mode.
- Logging of key events, errors, and debugging information.
- Management of CGI scripts and CGI mapping for specific locations.
- General and request caching mechanisms.

## Constructor
### `SocketHandler(int port, ServerConfig& config, const Logger* logger)`
The constructor initializes a socket, sets its options, binds it to the specified port, and configures it for listening. Additionally, it performs the following steps:
1. Checks if a valid logger pointer is provided; throws an exception if it's null.
2. Creates a socket using the `socket()` function.
3. Sets the socket option `SO_REUSEADDR` to allow the reuse of local addresses.
4. Binds the socket to the provided port.
5. Configures the socket to listen for incoming connections.
6. Sets the socket to non-blocking mode.
7. Maps CGI extensions (`.py`, `.pl`) to handle dynamic requests.

### Exceptions
- Throws `Logger::NoLoggerPointer` if the logger pointer is null.
- Throws `WebServerException` for errors during socket creation, setting socket options, binding, listening, or configuring the socket as non-blocking.

## Destructor
### `~SocketHandler()`
The destructor properly releases all resources used by the `SocketHandler`. Specifically, it calls the `close_socket()` method to close the socket file descriptor, ensuring no resources are leaked.

## Public Methods
### `int accept_connection()`
Accepts an incoming connection on the listening socket. If the connection is successful, it sets the client socket to non-blocking mode. If there's an error, it logs a warning message.
- **Returns**: The file descriptor for the accepted client connection, or -1 if an error occurs.

### `void close_socket()`
Closes the socket if it is still open. The socket file descriptor is first checked to ensure that it is valid and not closed. If any error occurs during closing, it is logged.

### `std::string get_port() const`
Returns the port number as a string.
- **Returns**: A string representing the port number.

### `int get_socket_fd() const`
Returns the file descriptor for the socket managed by the `SocketHandler`.
- **Returns**: The socket file descriptor.

### `const ServerConfig& get_config() const`
Returns a constant reference to the server configuration.
- **Returns**: A constant reference to the `ServerConfig` object.

### `bool set_nonblocking(int fd)`
Sets the specified socket file descriptor to non-blocking mode using the `fcntl()` system call.
- **Parameters**: `fd` - The file descriptor to be set as non-blocking.
- **Returns**: `true` if the socket was successfully set to non-blocking, otherwise `false`.

### `bool belongs_to_location(const std::string& path, const std::string& loc_root)`
Checks whether a given path belongs to a specific server location.
- **Parameters**:
    - `path` - The path to check.
    - `loc_root` - The root location to match.
- **Returns**: `true` if the path belongs to the specified location, otherwise `false`.

### `bool is_cgi_file(const std::string& filename, const std::string& extension)`
Determines if the given file has a specific CGI extension.
- **Parameters**:
    - `filename` - The name of the file to check.
    - `extension` - The CGI file extension to match.
- **Returns**: `true` if the filename ends with the specified extension, otherwise `false`.

### `void get_cgi_files(const std::string& directory, const std::string& loc_root, const std::string& extension, std::map<std::string, t_cgi>& mapped_files)`
Finds and maps CGI files within a given directory and its subdirectories.
- **Parameters**:
    - `directory` - The directory to search for CGI files.
    - `loc_root` - The root location to match.
    - `extension` - The CGI file extension to search for.
    - `mapped_files` - A map where the found CGI files are stored.

### `void mapping_cgi_locations(const std::string& extension)`
Maps CGI locations for the provided file extension. It iterates through server-configured locations and searches for CGI files, adding valid files to the CGI locations map.
- **Parameters**: `extension` - The CGI file extension to search for.

### `WebServerCache<CacheEntry>& get_cache()`
Returns a reference to the general cache used by the `SocketHandler`.
- **Returns**: A reference to the `WebServerCache` containing `CacheEntry` elements.

### `WebServerCache<CacheRequest>& get_request_cache()`
Returns a reference to the request cache used by the `SocketHandler`.
- **Returns**: A reference to the `WebServerCache` containing `CacheRequest` elements.

## Logging
The `SocketHandler` class extensively logs the following events:
- **Info**: General information such as successful socket creation, binding, listening, and CGI mapping.
- **Debug**: Detailed debugging information, including the start and completion of operations, and specific state changes.
- **Warning/Error**: Issues such as failed socket creation, errors in accepting connections, and problems in setting sockets to non-blocking mode.

## Usage
The `SocketHandler` class is used to manage server sockets effectively and handle incoming client connections. It is designed to be robust in error handling, providing detailed logs of its activities to aid in debugging and monitoring.
