/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mac <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 21:36:54 by mac               #+#    #+#             */
/*   Updated: 2024/10/17 21:36:55 by mac              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef __LOGGER_HPP__
# define __LOGGER_HPP__

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

enum e_logger {
	LOG_DEBUG = 0,
	LOG_INFO = 1,
	LOG_WARNING = 2,
	LOG_ERROR = 3
};

enum e_logger_module {
	LM_LOGGER = 0,
	LM_SERVER_MANAGER = 1,
	LM_SOCKET_MANAGER = 2,
	LM_REQUEST_MANAGER = 3
};

/**
 * @brief Logger class for logging messages to a file.
 *
 * This class provides functionality to log messages at different levels
 * (INFO, ERROR) into a specified log file.
 */
class Logger {
private:
	typedef void (Logger::*logging)(const std::string&, const std::string& ) const;
	int                         _level;
	bool                        _log_file;
	std::ofstream               _out_file;
	std::ostream*               _log_out;
	std::vector<logging>        _log;
	std::vector<std::string>    _modules;
public:
	Logger(bool log_to_file, std::vector<std::string> modules, int level);
	~Logger();
	void log(int module, int level, const std::string& message);
	void log_debug(const std::string& module, const std::string& message) const ;
	void log_info(const std::string& module, const std::string& message) const ;
	void log_warning(const std::string& module, const std::string& message) const ;
	void log_error(const std::string& module, const std::string& message) const ;
};

#endif


