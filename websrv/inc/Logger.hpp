/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 08:43:27 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/13 02:19:30 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef __LOGGER_HPP__
# define __LOGGER_HPP__

#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <sstream>

/**
 * @brief Logger level enum
 *
 * Inside class, logger level is used as a index for messages.
 * Using the enum, is easier to write code and know which level
 * any method is logging.
 */
enum e_logger {
	LOG_DEBUG = 0,
	LOG_INFO = 1,
	LOG_WARNING = 2,
	LOG_ERROR = 3
};

/**
 * @class Logger
 * @brief A simple logging utility for managing log messages with different levels of severity.
 *
 * This class provides methods for logging messages with different severity levels, including DEBUG, INFO, WARNING, ERROR, and FATAL.
 * It supports logging to the console or to a file, and allows for filtering messages based on a log level.
 */
class Logger {
private:
	int                         _level;
	bool                        _log_to_file;
	std::ofstream               _out_file;
	std::ostream*               _log_out;
	std::string 				_log_level[4];

public:
	Logger(int level, bool log_to_file);
	~Logger();
	template<typename T>
	void new_log(int level, const std::string& module, T message) const;
	void log(int level, const std::string& module, const std::string& message) const;
	void fatal_log(const std::string& module, const std::string& message) const;
	void status(const std::string& module, const std::string& message) const;
	void log_debug(const std::string& module, const std::string& message) const;
	void log_info(const std::string& module, const std::string& message) const;
	void log_warning(const std::string& module, const std::string& message) const;
	void log_error(const std::string& module, const std::string& message) const;

	class NoLoggerPointer : public std::exception {
		public:
			virtual const char *what() const throw();
    };
};

#endif


