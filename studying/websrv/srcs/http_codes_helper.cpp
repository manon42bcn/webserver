/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request_helpers.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mac <marvin@42.fr>                         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/16 23:33:40 by mac               #+#    #+#             */
/*   Updated: 2024/10/17 15:24:56 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __HTTP_CODES_HELPERS_CPP__
# define __HTTP_CODES_HELPERS_CPP__

#include "http_enum_codes.hpp"
#include "HttpRequestHandler.hpp"
#include <map>
#include <iostream>
// Método temporal, para facilitar el debug únicamente
std::string method_enum_to_string(int method)
{
	switch (method) {
		case METHOD_GET:
			return ("GET");
		case METHOD_POST:
			return ("POST");
		case METHOD_DELETE:
			return ("DELETE");
	}
	return ("NO METHOD");
}

/**
 * @brief Get a standard message for a http code
 *
 * @param code http code.
 * @return short standard message associated with the http code.
 */
std::string http_status_description(e_http_sts code)
{
	static std::map<int, std::string> http_codes;
	if (http_codes.empty()) {
		http_codes[HTTP_CONTINUE] = "Continue";
		http_codes[HTTP_SWITCHING_PROTOCOLS] = "Switching Protocols";
		http_codes[HTTP_PROCESSING] = "Processing";
		http_codes[HTTP_EARLY_HINTS] = "Early Hints";
		// 2xx Success
		http_codes[HTTP_OK] = "OK";
		http_codes[HTTP_CREATED] = "Created";
		http_codes[HTTP_ACCEPTED] = "Accepted";
		http_codes[HTTP_NON_AUTHORITATIVE_INFORMATION] = "Non-Authoritative Information";
		http_codes[HTTP_NO_CONTENT] = "No Content";
		http_codes[HTTP_RESET_CONTENT] = "Reset Content";
		http_codes[HTTP_PARTIAL_CONTENT] = "Partial Content";
		http_codes[HTTP_MULTI_STATUS] = "Multi-Status";
		http_codes[HTTP_ALREADY_REPORTED] = "Already Reported";
		http_codes[HTTP_IM_USED] = "IM Used";
		// 3xx Redirection
		http_codes[HTTP_MULTIPLE_CHOICES] = "Multiple Choices";
		http_codes[HTTP_MOVED_PERMANENTLY] = "Moved Permanently";
		http_codes[HTTP_FOUND] = "Found";
		http_codes[HTTP_SEE_OTHER] = "See Other";
		http_codes[HTTP_NOT_MODIFIED] = "Not Modified";
		http_codes[HTTP_USE_PROXY] = "Use Proxy";
		http_codes[HTTP_TEMPORARY_REDIRECT] = "Temporary Redirect";
		http_codes[HTTP_PERMANENT_REDIRECT] = "Permanent Redirect";
		// 4xx Client Error
		http_codes[HTTP_BAD_REQUEST] = "Bad Request";
		http_codes[HTTP_UNAUTHORIZED] = "Unauthorized";
		http_codes[HTTP_PAYMENT_REQUIRED] = "Payment Required";
		http_codes[HTTP_FORBIDDEN] = "Forbidden";
		http_codes[HTTP_NOT_FOUND] = "Not Found";
		http_codes[HTTP_METHOD_NOT_ALLOWED] = "Method Not Allowed";
		http_codes[HTTP_NOT_ACCEPTABLE] = "Not Acceptable";
		http_codes[HTTP_PROXY_AUTHENTICATION_REQUIRED] = "Proxy Authentication Required";
		http_codes[HTTP_REQUEST_TIMEOUT] = "Request Timeout";
		http_codes[HTTP_CONFLICT] = "Conflict";
		http_codes[HTTP_GONE] = "Gone";
		http_codes[HTTP_LENGTH_REQUIRED] = "Length Required";
		http_codes[HTTP_PRECONDITION_FAILED] = "Precondition Failed";
		http_codes[HTTP_CONTENT_TOO_LARGE] = "Payload Too Large";
		http_codes[HTTP_URI_TOO_LONG] = "URI Too Long";
		http_codes[HTTP_UNSUPPORTED_MEDIA_TYPE] = "Unsupported Media Type";
		http_codes[HTTP_RANGE_NOT_SATISFIABLE] = "Range Not Satisfiable";
		http_codes[HTTP_EXPECTATION_FAILED] = "Expectation Failed";
		http_codes[HTTP_I_AM_A_TEAPOT] = "I'm a teapot";
		http_codes[HTTP_MISDIRECTED_REQUEST] = "Misdirected Request";
		http_codes[HTTP_UNPROCESSABLE_ENTITY] = "Unprocessable Entity";
		http_codes[HTTP_LOCKED] = "Locked";
		http_codes[HTTP_FAILED_DEPENDENCY] = "Failed Dependency";
		http_codes[HTTP_TOO_EARLY] = "Too Early";
		http_codes[HTTP_UPGRADE_REQUIRED] = "Upgrade Required";
		http_codes[HTTP_PRECONDITION_REQUIRED] = "Precondition Required";
		http_codes[HTTP_TOO_MANY_REQUESTS] = "Too Many Requests";
		http_codes[HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE] = "Request Header Fields Too Large";
		http_codes[HTTP_UNAVAILABLE_FOR_LEGAL_REASONS] = "Unavailable For Legal Reasons";
		// 5xx Server Error
		http_codes[HTTP_INTERNAL_SERVER_ERROR] = "Internal Server Error";
		http_codes[HTTP_NOT_IMPLEMENTED] = "Not Implemented";
		http_codes[HTTP_BAD_GATEWAY] = "Bad Gateway";
		http_codes[HTTP_SERVICE_UNAVAILABLE] = "Service Unavailable";
		http_codes[HTTP_GATEWAY_TIMEOUT] = "Gateway Timeout";
		http_codes[HTTP_HTTP_VERSION_NOT_SUPPORTED] = "HTTP Version Not Supported";
		http_codes[HTTP_VARIANT_ALSO_NEGOTIATES] = "Variant Also Negotiates";
		http_codes[HTTP_INSUFFICIENT_STORAGE] = "Insufficient Storage";
		http_codes[HTTP_LOOP_DETECTED] = "Loop Detected";
		http_codes[HTTP_NOT_EXTENDED] = "Not Extended";
		http_codes[HTTP_NETWORK_AUTHENTICATION_REQUIRED] = "Network Authentication Required";
	}

	std::map<int, std::string>::const_iterator it = http_codes.find(code);
	if (it == http_codes.end())
		return ("No Info Associated");
	return (it->second);
}

e_methods method_string_to_enum(const std::string& method)
{
	static std::map<std::string, int> methods_codes;

	if (methods_codes.empty())
	{
		methods_codes["GET"] = METHOD_GET;
		methods_codes["POST"] = METHOD_POST;
		methods_codes["DELETE"] = METHOD_DELETE;
	}
	std::map<std::string, int>::const_iterator it = methods_codes.find(method);
	if (it == methods_codes.end())
		return (METHOD_ERR);
	return ((e_methods)it->second);
}

#endif
