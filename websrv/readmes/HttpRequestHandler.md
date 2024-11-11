
# HttpRequestHandler Class

## Overview
The `HttpRequestHandler` class is a core component of the web server, responsible for managing HTTP request parsing, validation, and directing appropriate responses. It integrates closely with `ClientData`, `Logger`, and other request handlers to process requests efficiently, handling standard, CGI, Range, and Multipart types.

## Key Components
- **Server Configuration**: Uses `_config` from `ServerConfig` to retrieve server settings.
- **Logging**: Utilizes `_log` from `Logger` for detailed event tracking and error handling.
- **Client Interaction**: Manages `_client_data` to track client connection status and manage timeouts.
- **Request Metadata**: Stores key request attributes and parsing details within `_request_data`.

## Workflow
The primary function of the class is `request_workflow()`, which follows these main steps:
1. **Parse and Validate Request Header**: Reads and analyzes HTTP headers to determine the request type and associated resources.
2. **Determine Path Type**: Recognizes if the request involves a CGI, static file, or directory resource.
3. **Content Handling**: Loads and manages body content based on the request type (chunked or standard).
4. **Request Processing**: Invokes specific handlers based on request attributes and ensures appropriate response generation.

## Methods

### Request Parsing and Validation
- **`read_request_header`**: Reads the incoming request header, checks its size, and detects the header-body delimiter.
- **`parse_header`**: Parses the header, extracting fields and ensuring a valid structure.
- **`parse_method_and_path`**: Identifies the HTTP method and requested path, validating path length and format.

### Content Handling
- **`load_content`**: Manages body loading based on content type (chunked or standard).
- **`load_content_chunks`**: Handles `Transfer-Encoding: chunked` requests, parsing individual chunks and accumulating data.
- **`load_content_normal`**: Processes content with `Content-Length`, ensuring complete retrieval.

### Request Processing
- **`handle_request`**: Dispatches the request to the appropriate handler (`HttpResponseHandler`, `HttpCGIHandler`, etc.) based on request attributes.
- **`validate_request`**: Ensures that request data conforms to expected formats based on HTTP method, content type, and additional attributes.

### Error Management
- **`turn_off_sanity`**: Deactivates processing if validation fails, logging error details and setting appropriate HTTP status.

## Dependencies
This class works in conjunction with:
- **Logger**: For logging all activity and errors.
- **ClientData**: Tracks connection and timeout status.
- **WebServerCache**: (optional) Utilizes caching for improved response times.
- **Handlers (HttpResponseHandler, HttpCGIHandler, etc.)**: Manages specific types of HTTP requests.
