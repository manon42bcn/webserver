#ifndef _HTTP_MULTIPART_HANDLER_HPP
#define _HTTP_MULTIPART_HANDLER_HPP

# include "WebServerResponseHandler.hpp"
#define MP_NAME "HttpMultipartHandler"

class HttpMultipartHandler : public WsResponseHandler {
private:
	std::vector<s_multi_part>	_multi_content;
public:
	HttpMultipartHandler(const LocationConfig *location,
						 const Logger *log,
						 ClientData* client_data,
						 s_request& request,
						 int fd);
	bool handle_request();
	bool handle_post();
	void parse_multipart_data();
	bool validate_payload();
	void get_file_content(int pid, int (&fd)[2]);

};

#endif
