/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpCGIHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 10:39:21 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/06 20:39:04 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HTTP_CGI_HANDLER_
#define _HTTP_CGI_HANDLER_

#include "WebServerResponseHandler.hpp"

class HttpCGIHandler : public WsResponseHandler {
	private:
	std::vector<char*>          _cgi_env;

	public:
		HttpCGIHandler(const LocationConfig *location,
							const Logger *log,
							ClientData* client_data,
							s_request& request,
							int fd) : WsResponseHandler(location, log, client_data,
														request, fd) {};
		~HttpCGIHandler();
		bool handle_request();
	private:
		bool cgi_execute();
		void get_file_content(int pid, int (&fd)[2]);
		std::vector<char*> cgi_environment();
		void free_cgi_env();
		bool send_response(const std::string &body, const std::string &path);
};

#endif
