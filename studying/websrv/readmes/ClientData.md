# ClientData Class

## Overview

The `ClientData` class manages individual client connections for a web server application. It encapsulates information about each client connection, such as file descriptors, connection timestamps, and activity status, while providing functionality to monitor and control the state of each client connection. This class supports timeout handling, connection deactivation, and allows for easy resource management, ensuring that each client connection is handled securely and efficiently.

## Different timeouts

Note that you can timeout a connection and a request.

1. Connection: When a request includes `Connection: keep-alive` the client fd will not be closed and the server will keep monitoring that fd. That can head the server to listen in a potentially not abandoned connections. The time between last connection and now is what will be timeout in this case, the client will be removed from active client map, and its fd will not be monitored anymore.
2. Request: The time between the request, and the server response at its different steps:
   - Time to read a request from socket.
   - Time to read CGI response.
   - Time to send a response to client.

### Key Features

- **Connection State Management**: Tracks if a client connection is active, open, or eligible for cleanup due to inactivity.
- **Timeout Handling**: Monitors client connections and enforces timeouts based on configurable thresholds.
- **Resource Cleanup**: Safely closes client connections and cleans up resources when no longer needed.
- **Logging**: Records key events and errors for each client connection using a logger instance, which aids in debugging and monitoring.

## 1. Constructor:

```cpp
ClientData(const SocketHandler* server, const Logger* log, int fd);
```

- **Purpose**: Initializes a new `ClientData` instance for a client connection, setting its initial state and recording the current timestamp.
- **Parameters**:
    - `server`: Pointer to the server's `SocketHandler`, managing client connections.
    - `log`: Pointer to a `Logger` instance for logging activities.
    - `fd`: File descriptor for the client’s socket, which is used to track client events.

## 2. Destructor

```cpp
~ClientData();
```

- **Purpose**: Destructor that safely closes the client's file descriptor if it is still open and logs the resource cleanup.
- **Exception Handling**: Catches any exceptions that occur while closing the file descriptor to ensure smooth resource cleanup.


## 3. `chronos_request`

```cpp
bool chronos_request();
```

- **Purpose**: Checks if the client connection has exceeded the request timeout.
- **Returns**: `true` if the client is still within the allowed timeout period; `false` otherwise.
- **Effect**: Marks the client as inactive if the timeout has been exceeded.

### 4. `timeout_connection`

```cpp
bool chronos_connection();
```

- **Purpose**: Checks if the client connection has exceeded the client timeout (a longer period than `chronos`).
- **Returns**: `true` if the client is within the timeout period; `false` otherwise.
- **Effect**: Marks the client as inactive if the timeout has been exceeded.

### 5. `chronos_reset`

```cpp
void chronos_reset();
```

- **Purpose**: Resets the client activity timestamp to the current time, effectively resetting the timeout.

### 6. `get_server`

```cpp
const SocketHandler* get_server();
```

- **Purpose**: Retrieves the server's `SocketHandler` associated with this client.
- **Returns**: Pointer to the `SocketHandler` instance.

### 7. `get_fd`

```cpp
struct pollfd get_fd();
```

- **Purpose**: Returns the `pollfd` structure for monitoring client events.
- **Returns**: A `pollfd` structure containing the client's file descriptor and polling events.

### 8. `deactivate`

```cpp
void deactivate();
```

- **Purpose**: Sets the client as inactive, making it eligible for cleanup.

### 9. `close_fd`

```cpp
void close_fd();
```

- **Purpose**: Safely closes the client’s file descriptor, marks it as inactive, and logs the closure.
- **Exception Handling**: Catches and logs any exceptions that occur during the closure process.

### 10. `keep_alive`

```cpp
bool keep_alive() const;
```

- **Purpose**: Checks if the client connection is marked as active.
- **Returns**: `true` if the client connection is active; `false` otherwise.

### 11. `keep_active`

```cpp
void keep_active();
```

- **Purpose**: Marks the client connection as active, ensuring it remains open and is not cleaned up.

