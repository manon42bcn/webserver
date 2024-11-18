# HttpAutoIndex Class

## Overview
The `HttpAutoIndex` class is a specialized response handler used to generate automatic directory listings for HTTP GET requests. It inherits from the `WsResponseHandler` class and provides functionality to create and send a formatted HTML response that lists the contents of a directory.

## Features
- Generates HTML pages for automatic directory indexing.
- Handles HTTP GET requests to list the contents of a directory.
- Inherits from `WsResponseHandler`, allowing integration with the web server's request and response handling flow.
- Logs the processing steps for better monitoring and debugging.

## Constructor
### `HttpAutoIndex(const LocationConfig* location, const Logger* log, ClientData& client_data, s_request& request, int fd)`
The constructor initializes an instance of `HttpAutoIndex` to handle directory indexing requests. It calls the base class `WsResponseHandler` constructor to initialize shared attributes.

- **Parameters**:
    - `location`: A pointer to the `LocationConfig` object containing location-specific settings.
    - `log`: A pointer to the `Logger` object for logging purposes.
    - `client_data`: A pointer to the `ClientData` object containing data for the current client.
    - `request`: A reference to the `s_request` structure containing the current request data.
    - `fd`: The file descriptor used for managing the connection.

- **Details**:
    - The constructor first calls the base class constructor (`WsResponseHandler`) to initialize common attributes such as `location`, `log`, `client_data`, and `request`.
    - A debug log message is generated to indicate successful initialization of the `HttpAutoIndex` handler.

## Public Methods

### `bool handle_request()`
Handles an HTTP GET request for auto-generating a directory index.

- **Returns**: `true` if the request is handled successfully and the response is sent, otherwise `false`.

- **Details**:
    - **Allowed Method Check**: Checks if the HTTP GET method is allowed for the current resource. If not, sets the response status to HTTP 403 (Forbidden) and sends an error response.
    - **Loading File Content**: Calls `get_file_content()` to load the directory content. If successful, sets the MIME type to `text/html` and sends the response.
    - **Error Handling**: If `get_file_content()` fails, sends an error response and returns `false`.

### `void get_file_content(std::string& path)`
Generates the content of an auto index for a given directory.

- **Parameters**: `path` - The path to the directory for which the auto index will be generated.

- **Details**:
    - **Open Directory**: Attempts to open the directory specified by the `path`. If the directory cannot be opened, sets the HTTP status to `HTTP_NOT_FOUND`.
    - **Generate HTML Content**: Constructs an HTML page listing all the files and directories in the specified directory. Each entry includes a link, size information, and last modified date.
    - **Close Directory**: After processing all entries, closes the directory and stores the generated HTML content in `_response_data.content`.
    - **Exception Handling**: If an error occurs during processing, logs a warning message, sets the HTTP status to `HTTP_INTERNAL_SERVER_ERROR`, and marks the response as unsuccessful.