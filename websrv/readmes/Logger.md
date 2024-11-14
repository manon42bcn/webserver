
# Logger Class

## Overview

The `Logger` class is a simple logging utility for managing log messages with different levels of severity. It supports logging to the console or to a file, and allows filtering of messages based on a specified log level.

### Log Levels Supported:
- **DEBUG**: Detailed information typically of interest only when diagnosing problems.
- **INFO**: General informational messages.
- **WARNING**: Indicates a potential problem or important situation to note.
- **ERROR**: Error events of considerable importance.
- **FATAL**: Critical errors that cause the program to terminate.

## Constructor

### `Logger(int level, bool log_to_file)`

- **Description**: Constructs a `Logger` object with a specified log level and optional file logging.
- **Parameters**:
    - `level`: The minimum log level for messages to be logged (e.g., `LOG_DEBUG`, `LOG_INFO`).
    - `log_to_file`: Boolean indicating whether to log messages to a file.
- **Behavior**: If file logging is enabled but the file cannot be opened, logging defaults to the console. If the provided log level is out of the allowed range (`LOG_DEBUG` to `LOG_ERROR`), the constructor will terminate the program.

## Destructor

### `~Logger()`

- **Description**: Destructor for the `Logger` object.
- **Behavior**: Closes the log file if it is open and cleans up any resources.

## Methods

### `void log(int log_level, const std::string& module, const std::string& message) const`

- **Description**: Logs a message with a specified log level.
- **Parameters**:
    - `log_level`: The severity level of the message (e.g., `LOG_DEBUG`, `LOG_INFO`).
    - `module`: The name of the module or component generating the log message.
    - `message`: The log message to be recorded.
- **Behavior**: The message is logged if the provided log level is equal to or higher than the configured log level for the logger.

### `void fatal_log(const std::string &module, const std::string &message) const`

- **Description**: Logs a fatal error message and terminates the program.
- **Parameters**:
    - `module`: The name of the module or component generating the log message.
    - `message`: The fatal error message to be recorded.

### `void status(const std::string &module, const std::string &message) const`

- **Description**: Logs a status message to the standard output.
- **Parameters**:
    - `module`: The name of the module or component generating the log message.
    - `message`: The status message to be recorded.

### `void log_debug(const std::string& module, const std::string& message) const`

- **Description**: Logs a debug-level message.
- **Parameters**:
    - `module`: The name of the module or component generating the log message.
    - `message`: The debug message to be recorded.
- **Behavior**: The message is logged if the current log level permits it.

### `void log_info(const std::string& module, const std::string& message) const`

- **Description**: Logs an info-level message.
- **Parameters**:
    - `module`: The name of the module or component generating the log message.
    - `message`: The informational message to be recorded.
- **Behavior**: The message is logged if the current log level permits it.

### `void log_warning(const std::string& module, const std::string& message) const`

- **Description**: Logs a warning-level message.
- **Parameters**:
    - `module`: The name of the module or component generating the log message.
    - `message`: The warning message to be recorded.
- **Behavior**: The message is logged if the current log level permits it.

### `void log_error(const std::string& module, const std::string& message) const`

- **Description**: Logs an error-level message.
- **Parameters**:
    - `module`: The name of the module or component generating the log message.
    - `message`: The error message to be recorded.
- **Behavior**: Error messages are always logged regardless of the configured log level.

## Exception Class

### `Logger::NoLoggerPointer`

- **Description**: Exception thrown when a null pointer to a `Logger` is used.
- **Methods**:
    - `const char* what(void) const throw()`: Returns a message indicating that the Logger pointer is null.
