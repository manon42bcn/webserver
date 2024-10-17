/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseHandler.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/14 11:33:32 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSEHANDLER_HPP
#define HTTPRESPONSEHANDLER_HPP

#include "webserver.hpp"
#include "http_enum_codes.hpp"
#include <string>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sys/socket.h>
/**
 * @brief Handle HTTP responses
 *
 * Encapsulate methods to handle HTTP responses.
 * Handle state codes, content and Headers for HTTP responses.
 */
class HttpResponseHandler {
public:
	void send_response(int client_socket, int status_code, const std::string& content);
	void send_error_page(int client_socket, int error_code);

private:
	std::string generate_headers(int status_code, size_t content_length);
	std::string get_status_message(int status_code);
};

#endif
