/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebserverException.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 08:43:27 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/06 08:43:27 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# ifndef _WEBSERVER_EXCEPTION_
#define _WEBSERVER_EXCEPTION_

#include <exception>
#include <string>

/**
 * @class WebServerException
 * @brief Custom exception class for handling errors within the web server project.
 *
 * This exception class provides a standard way to handle errors by allowing custom messages.
 * It inherits from `std::exception` and overrides the `what()` method to return a message.
 *
 * @details
 * - Can be initialized with either `std::string` or `const char*` for message flexibility.
 * - The destructor is `noexcept`, ensuring that it does not throw exceptions during destruction.
 *
 * ### Public Methods
 * - `WebServerException(const std::string& message)`: Constructor that accepts a custom error message.
 * - `WebServerException(const char* message)`: Constructor for C-string error messages.
 * - `const char* what() const throw()`: Returns the error message.
 *
 * @note Use this exception throughout the project to ensure consistent error handling.
 *
 * @example
 * ```
 * throw WebServerException("An unexpected error occurred.");
 * ```
 */
class WebServerException : public std::exception {
private:
	std::string _message;

public:
	explicit WebServerException(const std::string& message) : _message(message) {};
	explicit WebServerException(const char* message) : _message(message) {};
	explicit WebServerException() : _message("WebServer Error.") {};
	virtual ~WebServerException() throw() {};
	virtual const char* what() const throw() {
		return (_message.c_str());
	}
};

#endif
