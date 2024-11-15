# ServerManager Class

## Overview

`ServerManager` is a core class in the web server application responsible for handling server instances, managing client connections, and processing network events. It supports multiple server configurations, client timeouts, and network event handling through a `poll`-based event loop. Additionally, `ServerManager` provides robust error handling and logging to ensure the stability and maintainability of server operations.

## Key Features

- Initializes multiple server instances with specified configurations.
- Manages active client connections using `poll` for scalable event handling.
- Implements connection timeouts and client cleanup.
- Includes logging for server actions, errors, and status updates.
- Provides automatic resource cleanup upon shutdown or error.

## Class Members

### Private Members

- **_poll_fds**: Vector of `pollfd` structures representing all file descriptors monitored by `poll`.
- **_servers**: Map of `SocketHandler` pointers, each representing a server instance with file descriptors as keys.
- **_clients**: Map of active client connections with file descriptors as keys.
- **_log**: Pointer to a `Logger` instance for recording server activity.
- **_cache**: Pointer to a `WebServerCache` instance for caching responses.
- **_active**: Boolean indicating if the server is running.
- **_healthy**: Boolean representing the server's health status.
- **_timeout_index**: Map of client fd, using timeout timestamp as key.
- **_index_timeout**: Map of timeout timestamp, using client fd as key. This last two members are using to make timeout process more agile.

### Public Methods

- **ServerManager(std::vector<ServerConfig>& configs, const Logger* logger, WebServerCache* cache)**
- **~ServerManager()**
- **void run()**
- **void turn_off_server()**

### Private Methods

- **void clear_poll()**: Closes and clears all file descriptors from `_poll_fds`.
- **void add_server(int port, ServerConfig& config)**: Initializes and adds a new server instance.
- **void cleanup_invalid_fds()**: Removes invalid file descriptors from `_poll_fds`.
- **void new_client(SocketHandler* server)**: Accepts a new client connection from a server.
- **bool add_server_to_poll(int server_fd)**: Adds a server’s file descriptor to `_poll_fds`.
- **void remove_client_from_poll(t_client_it client_data, size_t& poll_index)**: Removes a client from `_clients` and `_poll_fds`.
- **bool process_request(size_t& poll_fd_index)**: Processes incoming requests from clients.
- **void timeout_clients()**: Removes clients that have timed out.
- **void clear_clients()**: Deallocates all active client resources.
- **void clear_servers()**: Deallocates all server instances.
- **bool turn_off_sanity(const std::string& detail)**: Logs a critical error and sets the server to inactive.
- **time_t ServerManager::timeout_timestamp()**: Returns a timeout timestamp in microsecs. Timeout timestamp is current time + CLIENT_LIFECYCLE

## Key Methods

### Initialization

- **ServerManager**: Initializes the manager by creating multiple server instances from the provided configurations and checking the validity of pointers (`logger`, `cache`).
- **add_server**: Creates and initializes a new server, adding it to `_servers` and monitoring it in `_poll_fds`.

### Event Loop

- **run**: Main event loop that manages client timeouts, polls for events, and processes requests. The loop exits when `_active` is `false` or an unrecoverable error occurs.
- **process_request**: Processes a request from a client. If processing fails, it logs and handles the error gracefully.

### Client and Server Management

- **new_client**: Accepts a new client connection and adds it to `_clients` and `_poll_fds`.
- **remove_client_from_poll**: Safely removes a client from `_clients` and `_poll_fds` and deletes its resources.

### Cleanup

- **clear_clients**: Iterates over all clients and releases their resources.
- **clear_servers**: Iterates over all servers and releases their resources.
- **clear_poll**: Closes all file descriptors in `_poll_fds` and clears the list.

### Error Handling

- **turn_off_sanity**: Shuts down the server in case of a critical error by setting `_healthy` to `false` and `_active` to `false`.

**Note**: All errors are handle withing each class, and the server will still work, however there a few cases when the server will shut down.
1. Errors with Logger, DataClient or Server pointers. The Workflow ensures that those kinds of errors are not reachable, so we can assume that if there are raised a healthy issues is affecting the environment.
2. Not handled exceptions. The only errors that can be caught at this point are be related to the environment healthy (instance class error, e.g.). 

## Logging and Error Messages

All critical operations and errors in `ServerManager` are logged using the `Logger` instance, providing detailed insights into server status and client interactions.

