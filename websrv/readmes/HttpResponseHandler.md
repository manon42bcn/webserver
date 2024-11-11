# HttpResponseHandler

## Overview

The `HttpResponseHandler` class is designed to manage HTTP responses in a web server environment, specifically optimized for handling static file content with the support of a caching mechanism. By utilizing a cache, the class can retrieve frequently requested files more efficiently, reducing disk I/O and improving response times for static resources.

## Key Features

- **HTTP Response Management**: Manages responses for HTTP requests, specifically handling static files.
- **Caching Support**: Uses a caching mechanism to store frequently accessed files, minimizing redundant I/O operations.
- **Integration with WsResponseHandler**: Extends the `WsResponseHandler` base class to inherit core response handling functionality.

### 1. Constructor

```cpp
HttpResponseHandler(const LocationConfig *location,
                    const Logger *log,
                    ClientData* client_data,
                    s_request& request,
                    int fd,
                    WebServerCache* cache);
```

- **Purpose**: Initializes an `HttpResponseHandler` instance, setting up the necessary configuration, logging, client data, request information, file descriptor, and caching instance.
- **Parameters**:
    - `location`: Pointer to the `LocationConfig` that holds server location configurations.
    - `log`: Pointer to the `Logger` for recording response handler activities.
    - `client_data`: Pointer to `ClientData` associated with the specific client.
    - `request`: Reference to an `s_request` structure containing details about the client request.
    - `fd`: File descriptor for the client connection, used for sending responses.
    - `cache`: Pointer to the `WebServerCache` for caching file content to optimize file retrieval.

### 2. `get_file_content` (Path-based)

```cpp
void get_file_content(std::string& path);
```

- **Purpose**: Attempts to retrieve the content of a specified file, using the cache if the content is already stored there. If the content is not cached, the method loads it from disk and adds it to the cache.
- **Parameter**:
    - `path`: Path to the file whose content is being requested.
- **Use Case**: This method is typically called when handling requests for static files, such as HTML, CSS, or image files. Using the cache improves performance for frequently accessed files.


## Caching and Performance

The `HttpResponseHandler` integrates with `WebServerCache` to store and retrieve static content, reducing I/O operations for commonly requested files. When a file is first requested, it is loaded from disk and stored in the cache for subsequent requests. If a file is already cached, it can be retrieved directly from memory, providing a significant performance boost for static resources.
