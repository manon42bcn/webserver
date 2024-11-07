#ifndef _HTTP_RANGE_HANDLER_HPP_
#define _HTTP_RANGE_HANDLER_HPP_

#include "WebServerResponseHandler.hpp"
#define RRH_NAME "HttpRangeHandler"

class HttpRangeHandler : public WsResponseHandler {
	public:
		HttpRangeHandler(const LocationConfig *location,
	                     const Logger *log,
	                     ClientData* client_data,
	                     s_request& request,
	                     int fd);
		~HttpRangeHandler();
		bool handle_request();
		bool handle_get();
		void get_file_content(std::string& path);
		void parse_content_range();
		bool validate_content_range(size_t file_size);
		void get_file_content(int pid, int (&fd)[2]);
};

#endif
