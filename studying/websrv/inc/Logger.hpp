/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mac <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 21:36:54 by mac               #+#    #+#             */
/*   Updated: 2024/10/18 13:24:20 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef __LOGGER_HPP__
# define __LOGGER_HPP__

#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>

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
 * (INFO, ERROR, etc) to a log file o std::cout.
 *
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
	void log(int level, const std::string& module, const std::string& message) const;
	void fatal_log(const std::string& module, const std::string& message) const;
};

#endif


