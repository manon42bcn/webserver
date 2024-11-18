# HttpCGIHandler Class

The `HttpCGIHandler` class is responsible for handling Common Gateway Interface (CGI) requests within the HTTP server. CGI enables the server to execute external programs or scripts (e.g., Python, PHP) to generate dynamic content for HTTP responses. This class is derived from `WsResponseHandler`, inheriting fundamental response handling methods and specializing in managing CGI-based responses.

## Class Overview

The `HttpCGIHandler` class executes CGI scripts, reads their output, and formats it as an HTTP response. It manages pipes, environment variables, and error handling to ensure proper execution and data transfer from the CGI program to the client.

### Key Features

- **CGI Environment Setup**: Configures the environment variables necessary for CGI script execution.
- **CGI Execution**: Executes the CGI script in a subprocess using fork and exec.
- **Response Parsing**: Parses the CGI program's output to construct a valid HTTP response.
- **Error Handling**: Manages errors such as malformed headers, missing content, timeouts, and pipe issues.

## Request Handling Flow

The `HttpCGIHandler` follows a structured flow to process CGI requests. Below is a breakdown of its primary methods and their responsibilities in the handling flow.

### 1. Constructor: `HttpCGIHandler(const LocationConfig *location, const Logger *log, ClientData& client_data, s_request& request, int fd)`

The constructor initializes the CGI handler by setting up essential configurations, including logging and the client data required for handling CGI execution.

- **Initialization**: Logs the handler's initialization.
- **Dependency Injection**: Inherits configurations from `WsResponseHandler` to manage request handling.

### 2. `handle_request()`

This method is the main entry point for handling the CGI request.

Note that this class does not handle separated methods (GET, POST, DELETE). Server responsibility is run the CGI script, serve it the request content and read its response.

- **CGI Execution**: Calls `cgi_execute()` to run the CGI script.
- **Response Validation**: Checks the response from the CGI program for a valid header and Content-Type.
- **Header Parsing**: Processes the response header for HTTP status and connection management.
- **Final Response**: Sends the response content to the client if valid; otherwise, sends an error.

### 3. `cgi_execute()`

Executes the CGI script by creating pipes for data exchange between the server and CGI program.

- **Pipe Creation**: Sets up pipes for CGI communication.
- **Fork and Exec**: Forks a new process and executes the CGI script in the child process.
- **Request Body Handling**: Writes the request body to the CGI input pipe if applicable.
- **Error Handling**: Manages pipe and fork errors, ensuring proper cleanup.

### 4. `get_file_content(int pid, int (&fd)[2])`

Reads the output of the CGI script from the pipe, handling timeouts and partial data reads.

- **Non-Blocking Read**: Uses `poll()` to handle non-blocking reads and detect timeouts.
- **Response Assembly**: Reads the CGI output, handling potential poll and read errors.
- **Process Termination**: Kills the CGI process if a timeout occurs.

### 5. `cgi_environment()`

Sets up the environment variables required for the CGI program, following the CGI specification.

- **Environment Variables**: Populates variables such as `REQUEST_METHOD`, `CONTENT_LENGTH`, `SCRIPT_NAME`, and `QUERY_STRING`.
- **Error Handling**: Catches memory allocation errors and ensures proper cleanup if a failure occurs.

### 6. `free_cgi_env()`

Frees the allocated environment variables after the CGI execution completes.

### 7. `send_response(const std::string &body, const std::string &path)`

Sends the CGI response body to the client. This method acts as a wrapper for `sender()` to format the response.

### Error Management

The `HttpCGIHandler` employs `turn_off_sanity()` to handle errors gracefully, setting appropriate HTTP status codes and logging messages. Common errors include:

- **HTTP 500 Internal Server Error**: For pipe, fork, or execution failures.
- **HTTP 502 Bad Gateway**: For malformed CGI responses.
- **HTTP 504 Gateway Timeout**: For CGI program timeouts.
