/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpMultipartHandler.hpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/07 23:48:37 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/08 23:53:48 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HTTP_MULTIPART_HANDLER_HPP
#define _HTTP_MULTIPART_HANDLER_HPP

# include "WebServerResponseHandler.hpp"
#define MP_NAME "HttpMultipartHandler"

/**
 * @struct s_multi_part
 * @brief Represents a multipart section within an HTTP request payload.
 *
 * Contains details about a multipart element such as the disposition, name,
 * filename, content type, data, and status.
 */
struct s_multi_part {
	std::string 	disposition;
	std::string 	name;
	std::string 	filename;
	std::string 	type;
	std::string 	data;
	e_content_type	data_type;
	e_http_sts      status;
	s_multi_part(std::string di, std::string n,
	             std::string fn, std::string t,
	             std::string d): disposition(di), name(n),
	                             filename(fn), type(t), data(d), data_type(CT_FILE),
	                             status(HTTP_CREATED) {};
	s_multi_part(std::string fn, e_content_type dt, std::string d):
			filename(fn), data(d), data_type(dt) {};
};


/**
 * @class HttpMultipartHandler
 * @brief Manages HTTP requests with multipart form data for processing and validation.
 *
 * `HttpMultipartHandler` extends `WsResponseHandler` to handle and process
 * HTTP POST requests containing multipart form data. It validates the payload,
 * manages file storage, and handles permission checks, making it suitable for
 * handling complex form submissions and file uploads.
 *
 * ### Primary Responsibilities
 * - Parses and validates multipart form-data payloads.
 * - Ensures secure handling of file uploads by checking permissions.
 * - Saves valid files to the designated server directory.
 *
 * ### Workflow
 * - `handle_request()`: Entrypoint method
 * - `handle_post()`: Processes POST requests specifically for multipart form data.
 * - `validate_payload()`: Confirms that the request meets content-length, content-type,
 *   and payload requirements.
 * - `parse_multipart_data()`: Splits and processes each multipart section, extracting
 *   headers and content.
 *
 */
class HttpMultipartHandler : public WsResponseHandler {
private:
	std::vector<s_multi_part>	_multi_content;
	bool handle_post();
	void parse_multipart_data();
	bool validate_payload();
	void get_file_content(int pid, int (&fd)[2]);
public:
	HttpMultipartHandler(const LocationConfig *location,
						 const Logger *log,
						 ClientData* client_data,
						 s_request& request,
						 int fd);
	bool handle_request();
};

#endif
