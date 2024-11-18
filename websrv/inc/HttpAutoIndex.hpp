/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpAutoIndex.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/17 22:06:14 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/18 00:41:47 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPAUTOINDEX_HPP
#define HTTPAUTOINDEX_HPP

#include "WebServerResponseHandler.hpp"
#define AI_NAME "HttpAutoIndex"

/**
 * @brief The `HttpAutoIndex` class handles the generation of an automatic
 * 		  index for a directory listing.
 *
 * This class inherits from `WsResponseHandler` and is responsible for providing
 * a response that contains a directory index when requested.
 * It includes methods for reading the content of a directory and generating
 * the appropriate response.
 */
class HttpAutoIndex : public WsResponseHandler {
	private:
		virtual void get_file_content(int pid, int (&fd)[2]);
		virtual void get_file_content(std::string& path);
	public:
		HttpAutoIndex (const LocationConfig *location,
					   const Logger *log,
					   ClientData& client_data,
					   s_request& request,
					   int fd);
		bool handle_request();
};

#endif
