# HttpRangeHandler Class

The `HttpRangeHandler` class in this project is designed to handle HTTP range requests, specifically for retrieving parts of a file based on a specified byte range. This class is part of a larger web server implementation and inherits from `WsResponseHandler`, allowing it to leverage generic response handling methods while specializing in managing range-based content delivery.

## Class Overview

The `HttpRangeHandler` class processes HTTP GET requests that include a "Range" header, enabling partial content delivery based on byte offsets. This functionality is often used to support resuming downloads, streaming media, or other scenarios where segmented data transfer is beneficial.

### Key Features

- **Range Header Parsing**: Interprets the "Range" header to extract the specified byte range.
- **Content Validation**: Validates the range request to ensure it falls within file bounds.
- **Content Retrieval**: Fetches and delivers only the requested segment of the file.
- **Error Handling**: Returns appropriate HTTP error responses when range requests are malformed or unsatisfiable.

## Request Handling Flow

The `HttpRangeHandler` follows a structured flow to process HTTP range requests. Below is a breakdown of its primary methods and their responsibilities in the handling flow.

### 1. Constructor: `HttpRangeHandler(const LocationConfig *location, const Logger *log, ClientData* client_data, s_request& request, int fd)`

The constructor initializes the range handler by setting up necessary configurations, such as location and logging. It takes a `LocationConfig`, `Logger`, `ClientData`, `s_request`, and file descriptor `fd` as parameters, which provide context for request processing.

- **Initialization**: Logs the initialization and inherits initial setup from `WsResponseHandler`.
- **Dependencies**: Stores references to the location configuration, client data, and logging instances.

### 2. `handle_request()`

This method is the entry point for processing the request. It checks if the request method is GET, as range requests are only valid with GET.

- **GET Method Check**: Ensures the request method is GET.
- **Delegation**: Calls `handle_get()` if valid; otherwise, disables sanity and sends an HTTP 400 error.

### 3. `get_file_content(std::string& path)`

This method retrieves the file content based on the specified range and file path.

- **Path Validation**: Ensures the file path is not empty.
- **Range Parsing**: Calls `parse_content_range()` to interpret the byte range.
- **File Operations**: Opens the file, checks file size, and validates the content range.
- **Partial Content Retrieval**: Reads the specified segment based on `_response_data.start` and `_response_data.end`.
- **Error Handling**: Catches I/O and file errors, disabling sanity as needed.

### 4. `parse_content_range()`

Parses the "Range" header to determine the byte range requested.

- **Range Scenarios**:
    - **Full Range**: Both start and end are specified.
    - **Starting Position Only**: Only the starting byte is specified.
    - **Ending Position Only**: Only the ending byte is specified, starting from the end of the file.
- **Error Handling**: If the range format is invalid, disables sanity with an HTTP 416 error.

### 5. `validate_content_range(size_t file_size)`

This method ensures that the specified range is within the file's bounds.

- **Boundary Adjustments**: Adjusts the end position if it exceeds file size, based on the range scenario.
- **Satisfiability Check**: Verifies that the start and end positions are within bounds. If not, disables sanity with an HTTP 416 error.
- **Log Output**: Logs the validated range for debugging.

### Error Management

Throughout the request handling process, `HttpRangeHandler` uses the `turn_off_sanity()` method to disable further processing if an error is encountered. Errors are logged, and appropriate HTTP error codes are sent back as responses. Common error codes include:

- **HTTP 400 Bad Request**: For invalid method usage.
- **HTTP 403 Forbidden**: For file access issues.
- **HTTP 404 Not Found**: For non-existent files.
- **HTTP 416 Range Not Satisfiable**: For unsatisfiable or malformed range headers.

### Example Scenario

Suppose a client requests a specific byte range, "bytes=0-499", for a video file:

1. **Range Parsing**: `parse_content_range()` reads the range "0-499" and stores it in `_response_data`.
2. **Range Validation**: `validate_content_range()` ensures the range is within the file size.
3. **Content Retrieval**: `get_file_content()` reads the first 500 bytes of the file.
4. **Response**: If successful, `send_response()` transmits the partial content to the client.
