```
@@@@@@@@@                                                                                                     
@@@@@@@@@@@@                                                                                                  
  @@@@@@@@@@@@@                                                                                               
        @@@@@@@@@                                                                                             
@@@@@@@@@@@@@@@@.....                                                                                         
@@@@@@@@@@@@@@+.::::..                                                                                        
@@@@@@@@@@@@@@..::::..                                                                                        
              .......=@                                                                                       
               @@*+%@@@@                                        @                                             
               @@@@@@@@@                                       @@@                                            
               @@@@@@@@@@                                      @@@@                                           
               @@@@@@@@@@                                      @@@@                                           
               @@@@@ @@@@@   @@@           @@@    @@@@@@@@     @@@@@@@@@@                                     
              @@@@@@ @@@@@  @@@@          @@@@   @@@@@@@@@@@   @@@@@@@@@@@                                    
             @@@@@@  @@@@@  @@@@          @@@@  @@@@@   @@@@   @@@@   @@@@@                                   
             @@@@@@  @@@@@  @@@@    @@@   @@@@  @@@@@@@@@@@@   @@@@    @@@@                                   
            @@@@@@   @@@@@  @@@@   @@@@   @@@@  @@@@@@@@@@@@   @@@@    @@@@                                   
          @@@@@@@   @@@@@   @@@@@  @@@@@  @@@@  @@@@@     @@   @@@@   @@@@@                                   
         @@@@@@@    @@@@@    @@@@@@@@@@@@@@@@@   @@@@@@@@@@@    @@@@@@@@@@                                    
       @@@@@@@@  ....*@@      @@@@@@@@@@@@@@@      @@@@@@@@@     @@@@@@@@                                     
     @@@@@@@@   ..:::..*                                                        @                             
...=@@@@@@@@ @@#..::::..     @@@@@@@@@@     @@@@@@@@      @@@@@@@@@  @@@@     @@@@@   @@@@@@@@      @@@@@@@@@ 
#+..=@@@@@@@@@@@..:::...    @@@@@@@@@@@    @@@@@@@@@@    @@@@@@@@@@@ @@@@@    @@@@@  @@@@@@@@@@@   @@@@@@@@@@@
%%#..@@@@@@@@@@@@%....      @@@@@@@@@@@   @@@@@@@@@@@@   @@@@   @@@@  @@@@   @@@@@  @@@@@@@@@@@@  @@@@@   @@@@
....@@@@@@#*@@@@@@@          @@@@@@@@@@   @@@@@@@@@@@@  @@@@          @@@@@  @@@@   @@@@@@@@@@@@  @@@@        
.*@@.......@@@@@@             @@@@@@@@@@  @@@@@@@@@@@@  @@@@           @@@@ @@@@@   @@@@@@@@@@@@  @@@@        
@@@=.-@@@=.+@@@              @@@    @@@@  @@@@@    @@   @@@@           @@@@@@@@@    @@@@@    @@@  @@@@        
@@@@..@@@...                @@@@@@@@@@@@   @@@@@@@@@@@  @@@@            @@@@@@@      @@@@@@@@@@@  @@@@        
@@@@#......                   @@@@@@@@@     @@@@@@@@@@   @@@             @@@@@        @@@@@@@@@@  @@@@

whoAmI(campus=42barcelona, login=mporras-, mail=manuel.porras.ojeda@gmail.com)
whoAmI(campus=42barcelona, login=vaguilar-, mail=veaz.24@gmail.com)
Feel free to write!  
```

# Web Server Project

This project implements a custom web server written in C++98, inspired by Nginx. It features the use of sockets, multiplexing, and several HTTP request handlers for a range of functionalities including static file serving, CGI execution, multipart handling, and more.

Note: This version has small differences with the full-subject version, to include some improvements that just have more sense. Keep that in mind ;-D

## Table of Contents
- [Introduction to Sockets and Multiplexing](#introduction-to-sockets-and-multiplexing)
- [Project Overview](#project-overview)
- [Core Components](#core-components)
- [Flow Description](#flow-description)
- [Building and Running](#building-and-running)
- [Configuration Parsing for Web Server](#configuration-parsing-for-web-server)
- [Generating Documentation with Doxygen](#generating-documentation-with-doxygen)

## Introduction to Sockets and Multiplexing

### Sockets
A **socket** is an endpoint for sending or receiving data across a network. It is a programming abstraction that allows processes to communicate over networks using standard protocols like TCP/IP. Sockets are integral to implementing servers and clients in networked applications.

- **Types of sockets**:
    - **Stream Sockets (TCP)**: Reliable, connection-based communication.
    - **Datagram Sockets (UDP)**: Unreliable, connectionless communication.

The server in this project uses TCP sockets to handle HTTP requests, ensuring data reliability.

### Multiplexing
Multiplexing allows a server to handle multiple connections efficiently without creating a dedicated thread for each connection. This server employs **poll** for I/O multiplexing to monitor multiple file descriptors simultaneously.

- **Why multiplexing?**
    - Scales better for high-concurrency workloads.
    - Reduces the overhead of thread context switching.
    - Enables asynchronous I/O operations.

## Project Overview

This web server is designed to process HTTP requests and respond efficiently using a non-blocking, event-driven architecture. Key features include:
- **Static file handling**: Serves files from a directory structure.
- **Dynamic content with CGI**: Executes scripts like Python or Perl.
- **HTTP range requests**: Supports partial file content delivery.
- **Multipart requests**: Processes form data with file uploads.

### Goals
- Create a lightweight, scalable server that adheres to HTTP specifications.
- Implement custom handlers for various HTTP methods (GET, POST, DELETE).
- Efficiently manage client connections using `poll` for multiplexing.

## Core Components

### ServerManager
Manages the server lifecycle, including:
- Initializing socket handlers.
- Managing client connections using `poll`.
- Distributing incoming requests to appropriate handlers.

### SocketHandler
Handles the setup and management of TCP sockets:
- Binding and listening on a port.
- Accepting new connections.
- Setting up non-blocking modes.

### HttpRequestHandler
Processes and validates incoming HTTP requests:
- Parses request headers, methods, and paths.
- Routes requests to appropriate response handlers.

### HttpResponseHandler
Constructs and sends HTTP responses:
- Handles static content delivery.
- Generates default and custom error pages.
- Manages content caching for efficiency.

### HttpCGIHandler
Executes CGI scripts and returns dynamic content based on the script's output.

### HttpMultipartHandler
Processes multipart form-data requests, commonly used for file uploads.

### Logger
Centralized logging for debug, info, warning, and error messages.

### WebServerCache
Implements caching mechanisms for static and dynamic content to reduce file system overhead.

## Flow Description

The server handles HTTP requests using an event-driven approach. Below is a detailed step-by-step flow of how a request is processed:

### 1. Server Initialization
- `ServerManager` initializes one or more `SocketHandler` instances based on the configuration file.
- Each `SocketHandler` sets up a TCP socket, binds it to a specific port, and enters listening mode.
- The sockets are configured as non-blocking to facilitate multiplexing.

### 2. Event Loop
- The `ServerManager` enters the main event loop, where it uses the `poll` system call to monitor multiple file descriptors.
- File descriptors are checked for readiness (e.g., new connections, incoming data, or write availability).

### 3. Handling New Connections
- When a new client connects, the `SocketHandler` accepts the connection and assigns it a new file descriptor.
- A `ClientData` object is created to manage the state and metadata of the client connection.

### 4. Processing Requests
- When incoming data is detected on a client’s file descriptor:
    1. The data is read and parsed by `HttpRequestHandler`.
    2. The request is validated, and key components such as HTTP method, headers, and body are extracted.
    3. The requested resource or action is determined based on the URL and method.

### 5. Routing Requests
- Depending on the HTTP method and request type:
    - **GET Requests**:
        - Static files are retrieved using `HttpResponseHandler`.
        - If the file exists, its content is sent to the client. If not, a 404 error is returned.
    - **POST Requests**:
        - Data is saved to the server or passed to a script for processing (e.g., file uploads or form submissions).
        - Errors like unsupported media types or size mismatches are handled gracefully.
    - **DELETE Requests**:
        - The specified resource is deleted, if permissions allow.
- Special handlers are used for:
    - **CGI Scripts**: `HttpCGIHandler` executes scripts and sends their output.
    - **Range Requests**: `HttpRangeHandler` serves partial file content.
    - **Multipart Requests**: `HttpMultipartHandler` processes form-data with file uploads.

### 6. Sending Responses
- The appropriate response handler constructs the HTTP response, including headers and body.
- The response is sent to the client using the non-blocking `send` system call.

### 7. Cleanup
- Disconnected or timed-out clients are removed from the event loop.
- Resources are released when the server shuts down.

## Building and Running

### Building
To build the project, use the provided Makefile:
```bash
make
```

### Running
Execute the server binary:
```bash
./webserver
```

### Dockerize Version
A quite simple Dockerize version is included. Please check that expose port and config file are the same.

# Configuration Parsing for Web Server

This module handles the parsing of configuration files for the web server. It reads the configuration file, validates its syntax and values, and converts it into structured objects used by the server during runtime.

## Table of Contents
- [Config Files Overview](#config-file-overview)
- [Parsing Workflow](#parsing-workflow)
- [Error Handling](#error-handling)
- [Available Configuration Options](#available-configuration-options)
- [Utilities and Validation](#utilities-and-validation)

## Config File Overview

The parsing system processes configuration files written in a specific format (e.g., `.conf`). The files define settings for servers, such as ports, root directories, error pages, and more. The parsing is modular, with distinct responsibilities for:
- Server-level configuration (e.g., port, root).
- Location-level configuration (e.g., specific directory settings).

The parsing logic ensures the configuration is valid and complete before starting the server.

## Parsing Workflow

1. **Reading Raw Lines**:
    - The configuration file is read line by line using `get_raw_lines`.
    - Lines are cleaned of whitespace and comments (`#`) for further processing.

2. **Parsing Blocks**:
    - Server blocks (`server {}`) are identified and parsed by `parse_server_block`.
    - Location blocks (`location {}`) within servers are parsed by `parse_location_block`.

3. **Assigning Parameters**:
    - Each recognized parameter is extracted and validated using specific parsing functions:
        - Example: `parse_port`, `parse_root`, `parse_autoindex`.
    - Nested structures like `locations` are constructed recursively.

4. **Post-Processing**:
    - Paths are normalized (e.g., removing extra slashes).
    - Default values are assigned for missing parameters.

## Error Handling

### Validation
Each parameter is validated using helper functions before being added to the configuration. Examples include:
- `check_port`: Ensures the port number is within the valid range (1–65535).
- `check_error_page`: Validates that error pages are associated with valid HTTP status codes and exist as files.
- `check_client_max_body_size`: Ensures sizes are correctly formatted (e.g., `1M`, `512K`).

### Logging and Fatal Errors
Errors encountered during parsing are logged with detailed messages using the `Logger`. Fatal errors (e.g., invalid syntax or missing required parameters) result in termination of the process.

#### Examples of Fatal Errors:
- Missing closing braces (`}`) in the configuration file.
- Duplicate port definitions in a server block.
- Invalid or unsupported configuration options.

## Available Configuration Options

### Server Block
Defines global settings for the server. Example syntax:
```conf
server {
    port 8080;
    server_name example.com;
    root /var/www/html;
    index index.html index.htm;
    error_page 404 /errors/404.html;
    client_max_body_size 1M;
    autoindex on;
    location /api {
        root /var/www/api;
        index index.py;
        autoindex off;
        cgi on;
        error_page 500 /errors/500.html;
    }
}
```

### Options

#### Global Options
- **`port`**: Port number for the server.
- **`server_name`**: Name of the server (default: `localhost`).
- **`root`**: Root directory for the server.
- **`index`**: Default file(s) to serve (e.g., `index.html`).
- **`error_page`**: Maps HTTP status codes to custom error pages.
- **`client_max_body_size`**: Maximum size of client request bodies (e.g., `1M`, `512K`).
- **`autoindex`**: Enables or disables directory indexing (`on`/`off`).

#### Location Block
Specifies settings for specific paths. Inherits options from the server block unless explicitly overridden.
- **`root`**: Root directory for this location.
- **`index`**: Default file(s) for this location.
- **`autoindex`**: Enables/disables directory indexing for this location.
- **`cgi`**: Enables/disables CGI execution (`on`/`off`).
- **`error_page`**: Custom error pages for this location.

## Utilities and Validation

The following helper functions are used for parsing and validation:

### Utilities
- **`split_error_pages`**: Converts a string of error pages into a map of status codes and paths.
- **`split_default_pages`**: Splits default page strings into a vector.
- **`get_raw_lines`**: Reads and cleans lines from the configuration file.

### Validation
- **`check_port`**: Ensures ports are valid.
- **`check_root`**: Validates root paths.
- **`check_autoindex`**: Ensures autoindex values are `on` or `off`.
- **`check_error_page`**: Ensures error pages map to valid paths and status codes.

## Example Configuration File
Below is an example of a complete configuration file:
```conf
server {
    port 80;
    server_name localhost;
    root /var/www/html;
    index index.html index.htm;
    error_page 404 /errors/404.html;
    client_max_body_size 2M;
    autoindex off;

    location /images {
        root /var/www/images;
        autoindex on;
    }

    location /cgi-bin {
        root /usr/lib/cgi-bin;
        cgi on;
        error_page 500 /errors/500.html;
    }
}
```
## Generating Documentation with Doxygen

To generate detailed, and more friendly documentation for this project, follow the steps outlined below:
Note: If you run the webserver with configs/main.conf, you can go to http://localhost:8080/docs/html/index.html

### 1. Install Doxygen:

If you haven't already installed Doxygen, you can do so using your system's package manager.

* Debian/Ubuntu-based systems:

```bash
sudo apt-get install doxygen
```

* On macOS:

```bash
brew install doxygen
```

### 2. Generate the Documentation:

From the root directory of the project, where the `Doxyfile` is located, run:

```bash
doxygen Doxyfile
```

### 3. View the Documentation:

Once Doxygen completes the documentation generation, you can view it by opening the `html/index.html` file in your preferred web browser:

```bash
open docs/html/index.html   # On macOS
xdg-open docs/html/index.html   # On Linux systems
```
