/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 11:20:38 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/13 02:21:27 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

/**
 * @brief Constructs a Logger object with a specified log level and optional file logging.
 *
 * This constructor initializes the logger with a specific log level, determining the minimum level of messages that will be logged.
 * If file logging is enabled, the logger attempts to open a log file; if the file cannot be opened, logging defaults to the console.
 *
 * @param level The minimum log level for messages to be logged (e.g., LOG_DEBUG, LOG_INFO).
 * @param log_to_file Boolean indicating whether to log messages to a file.
 *
 * @note If the provided log level is out of the allowed range (LOG_DEBUG to LOG_ERROR), the constructor will terminate the program.
 */
Logger::Logger(int level, bool log_to_file): _level(level), _log_to_file(log_to_file)
{
	if (_level > LOG_ERROR || _level < LOG_DEBUG) {
		std::cerr << "[LOGGER][ERROR]. Log level off limits (0-3)." << std::endl;
		exit(1);
	}
	_log_out = &std::cout;
	if (_log_to_file)
	{
		std::string out_file = "Logger_logs.log";
		_out_file.open(out_file.c_str(), std::ios::binary);
		if (!_out_file.is_open()) {
			_log_to_file = false;
			*(_log_out) << "[LOGGER][WARNING] logfile.log cannot be open. Logs will be show at std::cout.\n";
		} else {
			_log_out = &_out_file;
		}
	}
	// load log level strings
	_log_level[LOG_DEBUG] = "[DEBUG][";
	_log_level[LOG_INFO] = "[INFO][";
	_log_level[LOG_WARNING] = "[WARNING][";
	_log_level[LOG_ERROR] = "[ERROR][";
	*(_log_out) << "[LOGGER][INFO] Logger init and ready.\n";
}

/**
 * @brief Destructor for the Logger object.
 *
 * Closes the log file if it is open and cleans up any resources.
 */
Logger::~Logger() {
	if (_out_file.is_open()) {
		_out_file.close();
	}
	_log_out = NULL;
}

/**
 * @brief Logs a message with a specified log level.
 *
 * This function logs a message if the provided log level is equal to or higher than the configured log level for the logger.
 *
 * @param log_level The severity level of the message (e.g., LOG_DEBUG, LOG_INFO).
 * @param module The name of the module or component generating the log message.
 * @param message The log message to be recorded.
 */
void Logger::log(int log_level, const std::string& module, const std::string& message) const{
	if (log_level < LOG_DEBUG && log_level > LOG_ERROR) {
		*(_log_out) << _log_level[LOG_ERROR] << "[LOGGER]: Log level out of range." << std::endl;
		return;
	} else {
		if (log_level >= _level)
			*(_log_out) << _log_level[log_level] << module << "]: " << message << std::endl;
		return;
	}
}

/**
 * @brief Logs a fatal error message and terminates the program.
 *
 * This function logs a fatal error message, prints it to the standard error output, and then terminates the program.
 *
 * @param module The name of the module or component generating the log message.
 * @param message The fatal error message to be recorded.
 */
 void Logger::fatal_log(const std::string &module, const std::string &message) const {
	*(_log_out) << "[FATAL ERROR][" << module << "]: " << message << std::endl;
	std::cerr << "[FATAL ERROR][" << module << "]: " << message << std::endl;
	exit(1);
 }

/**
* @brief Logs a status message to the standard output.
*
* This function logs a status message, typically used for indicating non-error status updates.
*
* @param module The name of the module or component generating the log message.
* @param message The status message to be recorded.
*/
void Logger::status(const std::string &module, const std::string &message) const {
	 std::cout << "[" << module << "]" << message << std::endl;
 }

/**
* @brief Logs a debug-level message.
*
* This function logs a message with DEBUG severity if the current log level permits it.
*
* @param module The name of the module or component generating the log message.
* @param message The debug message to be recorded.
*/
void Logger::log_debug(const std::string& module, const std::string& message) const {
	if (_level > LOG_DEBUG) {
		return;
	}
	*(_log_out) << "[DEBUG][" << module << "]: " << message << std::endl;
 }

/**
* @brief Logs an info-level message.
*
* This function logs a message with INFO severity if the current log level permits it.
*
* @param module The name of the module or component generating the log message.
* @param message The informational message to be recorded.
*/
void Logger::log_info(const std::string& module, const std::string& message) const {
	if (_level > LOG_INFO) {
		return;
	}
	*(_log_out) << "[INFO][" << module << "]: " << message << std::endl;
 }

/**
* @brief Logs a warning-level message.
*
* This function logs a message with WARNING severity if the current log level permits it.
*
* @param module The name of the module or component generating the log message.
* @param message The warning message to be recorded.
*/
void Logger::log_warning(const std::string& module, const std::string& message) const {
	if (_level > LOG_WARNING) {
		return;
	}
	*(_log_out) << "\033[0;93m[WARNING]\033[0;39m[" << module << "]: " << message << std::endl;
 }

/**
* @brief Logs an error-level message.
*
* This function logs a message with ERROR severity. Error messages are always logged regardless of the configured log level.
*
* @param module The name of the module or component generating the log message.
* @param message The error message to be recorded.
*/
void Logger::log_error(const std::string& module, const std::string& message) const {
	*(_log_out) << "\033[31m[ERROR]\033[0;39m[" << module << "]: " << message << std::endl;
 }

/**
* @class Logger::NoLoggerPointer
* @brief Exception thrown when a null pointer to a Logger is used.
*
* This exception is used to indicate that a null pointer was passed where a valid Logger pointer was expected.
*/

/**
 * @brief Returns a message indicating that the Logger pointer is null.
 *
 * @return A constant character pointer with an error message.
 */
const char *Logger::NoLoggerPointer::what(void) const throw() {
	return ("Logger Pointer cannot be NULL.");
}

