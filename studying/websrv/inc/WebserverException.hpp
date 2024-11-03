
# ifndef _WEBSERVER_EXCEPTION_
#define _WEBSERVER_EXCEPTION_

#include <exception>
#include <string>

class WebServerException : public std::exception {
private:
	std::string _message;

public:
	explicit WebServerException(std::string message) : _message(message) {};
	explicit WebServerException(const char* message) : _message(message) {};
	explicit WebServerException() : _message("WebServer Error.") {};
	virtual ~WebServerException() throw() {};
	virtual const char* what() const throw() {
		return (_message.c_str());
	}
};

#endif
