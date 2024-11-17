/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpAutoIndex.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/17 22:06:14 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/18 00:33:42 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPAUTOINDEX_HPP
#define HTTPAUTOINDEX_HPP

#include "WebServerResponseHandler.hpp"
#define AI_NAME "HttpAutoIndex"

class HttpAutoIndex : public WsResponseHandler {

	virtual void get_file_content(int pid, int (&fd)[2]);
	virtual void get_file_content(std::string& path);
public:
	HttpAutoIndex (const LocationConfig *location,
				   const Logger *log,
				   ClientData* client_data,
				   s_request& request,
				   int fd);
//	bool handle_get();
	bool handle_request();
};

#endif
