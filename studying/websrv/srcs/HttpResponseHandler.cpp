/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseHandler.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/14 13:50:15 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponseHandler.hpp"

/**
 * @brief Send a HTTP response with a personalized content
 *
 * @param client_socket Client Socket FD.
 * @param status_code state code.
 * @param content Payload of the response as string.
 */
void HttpResponseHandler::send_response(int client_socket, int status_code, const std::string& content) {
	std::string headers = generate_headers(status_code, content.length());
	std::string full_response = headers + content;

	send(client_socket, full_response.c_str(), full_response.length(), 0);
}

/**
 * @brief Send a error page.
 *
 * @param client_socket Client Socket FD.
 * @param error_code HTTP status code
 */
void HttpResponseHandler::send_error_page(int client_socket, int error_code)
{

	std::string error_message = get_status_message(error_code);
	std::string content = "<html><body><h1>" + error_message + "</h1></body></html>";
	send_response(client_socket, error_code, content);
}

/**
 * @brief Generates a HTTP header to a given status code.
 *
 * @param status_code HTTP state (status) code.
 * @param content_length len of the response.
 * @return std::string HTTP response header.
 */
std::string HttpResponseHandler::generate_headers(int status_code, size_t content_length) {
	std::string status_message = get_status_message(status_code);

	std::string headers =
			"HTTP/1.1 " + int_to_string(status_code) + " " + status_message + "\r\n" +
			"Content-Length: " + int_to_string(content_length) + "\r\n" +
			"Content-Type: text/html\r\n" +
			"Connection: close\r\n\r\n";

	return (headers);
}

/**
 * @brief Returns an message associate with a status code.
 *
 * @param status_code status code.
 * @return std::string verbose message.
 */
std::string HttpResponseHandler::get_status_message(int status_code) {
	switch (status_code) {
		case 200: return "OK";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		default:  return "Internal Server Error";
	}
}
