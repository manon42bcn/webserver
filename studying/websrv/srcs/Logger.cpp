/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mac <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 21:37:14 by mac               #+#    #+#             */
/*   Updated: 2024/10/17 21:37:16 by mac              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "Logger.hpp"
#include <iostream>

// Para incluir en main y debuguear...
//std::vector<std::string> modules;
//modules.push_back("Saludo");
//modules.push_back("Hello");
//
//Logger testLog(false, modules, 0);
//testLog.log(0, LOG_ERROR, "esta es una prueba para error");
//testLog.log(1, LOG_DEBUG, "esta es una prueba para hello");
/**
 * @brief Constructor that opens a log file for writing.
 *
 * Opens the specified log file in append mode. If the file cannot be opened,
 * an error message is printed to the standard error output.
 *
 * @param filename Name of the log file.
 * @warning if the file cannot be open - exit -
 */
Logger::Logger(bool log_to_file, std::vector<std::string> modules, int level): _level(level), _log_file(log_to_file)
{
	if (_level > LOG_ERROR || _level < LOG_DEBUG) {
		std::cerr << "Logger init error. Log level off limits (0-3)." << std::endl;
		exit(1);
	}
	if (modules.empty()) {
		std::cerr << "Logger init error. Empty modules vector." << std::endl;
		exit(1);
	}
	if (_log_file)
	{
		std::string out_file = "logfile.log";
		_out_file.open(out_file.c_str(), std::ios::binary);
		if (!_out_file.is_open()) {
			_log_file = false;
			_log_out = &std::cout;
			log_warning("LOGGER", "logfile.log cannot be open. Logs will be show at std::cout.");
		} else {
			_log_out = &_out_file;
		}
	}
	// Load descriptor of each module.
	for (size_t i = 0; i < modules.size(); i++)
		_modules.push_back(modules[i]);
	// Load logger functions
	_log.push_back(&Logger::log_debug);
	_log.push_back(&Logger::log_info);
	_log.push_back(&Logger::log_warning);
	_log.push_back(&Logger::log_error);
	log_info("LOGGER", "init complete.");
}

/**
 * @brief Destructor that closes the log file.
 *
 * If the log file is open, it is closed before the object is destroyed.
 */
Logger::~Logger() {
	if (_out_file.is_open()) {
		_out_file.close();
	}
}

/**
 * @brief Logs a generic message to the log file.
 *
 * The message is written directly to the log file, followed by a newline character.
 *
 * @param message The message to be logged.
 */
void Logger::log(int module, int level, const std::string& message) {
	if (level >= _level && (level >= LOG_DEBUG && level <= LOG_ERROR)) {
		(this->*_log[level])(_modules[module], message);
		return ;
	}
}

/**
 * @brief Logs an informational message to the log file.
 *
 * Prepends "[INFO]" to the message and writes it to the log file.
 *
 * @param message The informational message to be logged.
 */
void Logger::log_info(const std::string& module, const std::string& message) const {
	(*_log_out) << "[INFO] [" + module + "]: " + message;
}

void Logger::log_debug(const std::string &module, const std::string &message) const {
	(*_log_out) << "[DEBUG] [" + module + "]: " + message;
}
void Logger::log_warning(const std::string &module, const std::string &message) const {
	(*_log_out) << "[WARNING] [" + module + "]: " + message;
}

void Logger::log_error(const std::string& module, const std::string& message) const {
	(*_log_out) << "[ERROR] [" + module + "]: " + message;
}


