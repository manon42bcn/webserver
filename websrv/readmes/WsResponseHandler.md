
# WsResponseHandler Class

## Overview
`WsResponseHandler` is a base class for handling HTTP responses within a web server environment. It is designed to support common HTTP methods (GET, POST, DELETE) and manages file reading, content validation, response headers, error handling, and client interactions. Derived classes can extend `WsResponseHandler` to implement specialized behaviors for different response types, such as handling multipart data or CGI responses.

## Class Members

### Enumerations
- **`e_content_type`**: Defines content types handled in HTTP responses.
    - `CT_UNKNOWN`: Undefined or unknown content type.
    - `CT_FILE`: Content represents a file.
    - `CT_JSON`: Content is in JSON format.

- **`e_range_scenario`**: Describes byte-range request scenarios.
    - `CR_INIT`: Initial range request without specific end byte.
    - `CR_RANGE`: Specific byte range is requested.
    - `CR_LAST`: Last bytes of the content are requested.

### Structures
- **`s_content`**: Holds data associated with HTTP response content.
    - `ranged` (bool): Indicates if the response is a ranged request.
    - `start` (size_t): Start byte position for ranged content.
    - `end` (size_t): End byte position for ranged content.
    - `filesize` (size_t): Total size of the content file.
    - `range_scenario` (`e_range_scenario`): Range handling scenario.
    - `status` (bool): Success/failure status of content loading.
    - `content` (std::string): Actual content data for the response.
    - `mime` (std::string): MIME type of the content.
    - `http_status` (`e_http_sts`): HTTP status code.
    - `header` (std::string): Response header associated with the content.

## Class Definition

### Private Members
- **`int _fd`**: File descriptor for the client connection.
- **`std::string _content`**: Content of the response.
- **`std::vector<std::string> _response_header`**: Stores headers for the response.

### Protected Members
- **`const LocationConfig* _location`**: Location-specific configuration for requests.
- **`const Logger* _log`**: Logger instance for logging operations.
- **`ClientData& _client_data`**: Data for the current client request.
- **`s_request& _request`**: Reference to the request data.
- **`s_content _response_data`**: Contains attributes related to the response.
- **`std::string _headers`**: Complete headers for the response.

### Methods

#### Constructor & Destructor
- **`WsResponseHandler(const LocationConfig *location, const Logger *log, ClientData& client_data, s_request& request, int fd)`**: Initializes the response handler with location, logger, client data, request data, and file descriptor.
- **`virtual ~WsResponseHandler()`**: Destructor for cleanup.

#### Core Methods
- **`virtual bool handle_request()`**: Processes the HTTP request, generating an appropriate response based on the method.
- **`virtual bool handle_get()`**: Handles GET requests. (Overridable)
- **`virtual bool handle_post()`**: Handles POST requests. (Overridable)
- **`bool handle_delete()`**: Handles DELETE requests.
- **`virtual bool validate_payload()`**: Validates the payload in the request.
- **`virtual void get_file_content(int pid, int (&fd)[2])`**: Retrieves file content from specified process ID and file descriptor (pure virtual).
- **`virtual void get_file_content(std::string& path)`**: Retrieves file content from a given path.
- **`bool save_file(const std::string& save_path, const std::string& content)`**: Saves provided content to a specified file path.
- **`virtual std::string header(int code, size_t content_size, std::string mime)`**: Constructs the response header based on status code, content size, and MIME type.
- **`virtual bool send_response(const std::string& body, const std::string& path)`**: Sends the full HTTP response to the client.
- **`bool sender(const std::string& body)`**: Sends response data over the socket.
- **`std::string default_plain_error()`**: Generates a default error page in HTML format.
- **`bool send_error_response()`**: Sends a pre-defined error response.
- **`void turn_off_sanity(e_http_sts status, std::string detail)`**: Disables further processing if an error occurs, logging the error and setting the response status.

## Logging
The `WsResponseHandler` utilizes the logger instance (`Logger* _log`) for logging all operations, from request handling and payload validation to error handling and response sending.

## Error Handling
Error handling is facilitated by the `turn_off_sanity` and `send_error_response` methods, which stop further processing, set appropriate HTTP status codes, and log the details.

