
# HttpMultipartHandler Class

## Class Overview

The `HttpMultipartHandler` handles multipart form-data HTTP requests, particularly focused on processing and saving file uploads through the `POST` method. This class inherits from `WsResponseHandler` and extends its functionality to handle multipart data uploads, parsing individual parts, validating payloads, and managing access permissions.

Note that, in WebServer implementation, multipart form-data HTTP requests that point to a CGI script are handled separately.

## Key Features

- **Request Handling**: Processes multipart `POST` requests and saves uploaded file data.
- **Validation**: Ensures that each multipart upload meets required conditions, including content length, content type, and boundary presence.
- **Error Handling**: Deactivates the request sanity and sends error responses for invalid or incomplete multipart data.
- **Logging**: Logs each step of the multipart handling process, providing detailed information for debugging.


## Request Handling Flow

The `HttpMultipartHandler` follows a structured flow parse and process multipart requests.

### 1. Constructor: `HttpMultipartHandler(const LocationConfig *location, const Logger *log, ClientData* client_data, s_request& request, int fd)`

- **Initialization**: Logs the handler's initialization.
- **Dependency Injection**: Inherits configurations from `WsResponseHandler` to manage request handling.

### 2. `handle_request()` 

This method is the main entry point for handling Multi-part requests.

- **Sanity Check**: The request sanity is checked to ensure the multipart data is valid before proceeding.
- **Method Validation**: Only `POST` requests are permitted for multipart data handling. Non-`POST` methods result in a `HTTP_BAD_REQUEST` response.
- **Access Validation**: Confirms that the server location has the necessary write access (`ACCESS_WRITE`) for saving uploaded files.
- **Payload Validation**: The payload is validated for content length, content type, and boundary presence.

### 3. `handle_post()`
- **Validate Request**: Calls `validate_payload()` to load and parse multipart included at request body.
- **Try save each part**: Using `save_file` to handle each parsed multipart.

### 4. `validate_payload()`

- **Validate Request**: Validates the request payload, checking content length, content type, and body presence. It also verifies that the content length matches the body size.

### **5. `parse_multipart_data()`**

The `parse_multipart_data` method extracts individual parts of the multipart body. For each part:
- **Content-Disposition**: Parses content headers to retrieve file metadata such as `filename` and `content-type`.
- **File Saving**: Each file part is saved to the specified directory using the `save_file` method.

### Error Handling

- **HTTP_BAD_REQUEST**: For invalid method usage.
- **HTTP_FORBIDDEN**: For access permissions errors.
- **HTTP_MULTI_STATUS**: Is not an error code, but indicates that the request was partially satisfied. Workflows stops when an error if found.
- **HTTP_LENGTH_REQUIRED**: Length is required with POST method.
- **HTTP_UNSUPPORTED_MEDIA_TYPE**: For attempts to POST non valid types (or types included at black list, such a .py, .sh, etc.).