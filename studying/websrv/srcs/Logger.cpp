/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 11:20:38 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/18 13:24:00 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

/**
 * @brief Constructor of Logger instance
 *
 * @note this constructor will set:
 * 		 1.- Log level.
 * 		 2.- Log to file (bool).
 * 		 3.- _out_file. ofstream to save logs to a file (if log_to_file = true).
 * 		 4.- _log_out. Pointer to ostream (to _out_file or std::cout).
 * 		 5.- _log_level. Array with log level to prepend to log.
 *
 * @param level minimum level of a log to be logged to the output
 * @param log_to_file a boolean to set the log output.
 * 		  True will use a file to log (by default a logfile.log just next the bin).
 * 		  False will use std::cout to log.
 * @warning if a Logger_logs.log file exist, it will be overwritten at new execution.
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
 * @brief Destructor that closes the log file.
 *
 * If the log file is open, it is closed before the object is destroyed.
 * _log_out to NULL to avoid further errors.
 */
Logger::~Logger() {
	if (_out_file.is_open()) {
		_out_file.close();
	}
	_log_out = NULL;
}

/**
 * @brief Public method to log a message.
 *
 * Entrypoint to create a log.
 * @note the workflow is:
 * 		1.- The module sends a log, given: level, module and message.
 * 		2.- Log level is validated to avoid further errors
 * 		3.- The log is created.
 * @warning If a log level is below that the set level, the message will be ignore
 * 			and the log will be lost.
 * 			If a log level is lower or higher that the accepted levels, an error
 * 			log from Logger module will be included and the log will be lost.
 *
 * @param log_level Log level.
 * @param module Name of the module that is logging.
 * @param message Detail of the log.
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
 * @brief Public method to log an error message and exit from current execution.
 *
 * @warning this method will log and FATAL ERROR log, and exit(1).
 * 			It will ignore debug level set.
 * 			It will also shown the error message using std::cerr
 *
 * @param module Name of the module that is logging.
 * @param message Detail of the log.
 */
 void Logger::fatal_log(const std::string &module, const std::string &message) const {
	*(_log_out) << "[FATAL ERROR][" << module << "]: " << message << std::endl;
	std::cerr << "[FATAL ERROR][" << module << "]: " << message << std::endl;
	exit(1);
 }
