/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_codes_helper.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 15:08:04 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/23 00:02:25 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_codes_helper.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 10:22:22 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/13 23:13:51 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __HTTP_CODES_HELPERS_CPP__
# define __HTTP_CODES_HELPERS_CPP__

#include "http_enum_codes.hpp"
#include "HttpRequestHandler.hpp"
#include <map>
#include <iostream>
#include <set>
// Método temporal, para facilitar el debug únicamente
std::string method_enum_to_string(int method)
{
	switch (method) {
		case MASK_METHOD_GET:
			return ("GET");
		case MASK_METHOD_POST:
			return ("POST");
		case MASK_METHOD_DELETE:
			return ("DELETE");
		case MASK_METHOD_PUT:
			return ("PUT");
		case MASK_METHOD_HEAD:
			return ("HEAD");
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
		http_codes[HTTP_CLIENT_CLOSE_REQUEST] = "Client Close Request";
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

t_methods method_string_to_enum(const std::string& method)
{
	static std::map<std::string, t_methods> methods_codes;
	if (methods_codes.empty())
	{
		methods_codes["GET"] = MASK_METHOD_GET;
		methods_codes["POST"] = MASK_METHOD_POST;
		methods_codes["DELETE"] = MASK_METHOD_DELETE;
		methods_codes["PUT"] = MASK_METHOD_PUT;
		methods_codes["HEAD"] = MASK_METHOD_HEAD;
		methods_codes["OPTIONS"] = MASK_METHOD_OPTIONS;
		methods_codes["PATCH"] = MASK_METHOD_PATCH;
	}
	std::map<std::string, t_methods>::const_iterator it = methods_codes.find(method);
	if (it == methods_codes.end())
		return (0);
	return (it->second);
}

/**
 * @brief Creates and returns a map of file extensions to MIME types.
 *
 * This method generates a map that associates common file extensions (e.g., ".html", ".jpg")
 * with their corresponding MIME types (e.g., "text/html", "image/jpeg"). The map is used
 * to determine the `Content-Type` header when serving files.
 *
 * @details
 * - The method ensures that the map is initialized only once, using a static map to avoid
 *   recreating the map on each call. If additional MIME types are required, they can be
 *   added to the map.
 * - Common MIME types such as `text/html`, `application/javascript`, and `image/jpeg` are included.
 *
 * @return std::map<std::string, std::string> A map that associates file extensions with their MIME types.
 */
std::map<std::string, std::string> create_mime_types() {
	std::map<std::string, std::string> mime_types;
	// Text and Web
	mime_types[".html"] = "text/html";
	mime_types[".css"] = "text/css";
	mime_types[".js"] = "application/javascript";
	mime_types[".json"] = "application/json";
	mime_types[".txt"] = "text/plain";
	// Images
	mime_types[".jpg"] = "image/jpeg";
	mime_types[".jpeg"] = "image/jpeg";
	mime_types[".png"] = "image/png";
	mime_types[".gif"] = "image/gif";
	mime_types[".webp"] = "image/webp";
	mime_types[".svg"] = "image/svg+xml";
	// Audio
	mime_types[".mp3"] = "audio/mpeg";
	mime_types[".wav"] = "audio/wav";
	mime_types[".ogg"] = "audio/ogg";
	// Video
	mime_types[".mp4"] = "video/mp4";
	mime_types[".ogg"] = "video/ogg";  // Ogg can be both audio and video
	// Documents
	mime_types[".pdf"] = "application/pdf";
	mime_types[".doc"] = "application/msword";
	mime_types[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	mime_types[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	// Fonts
	mime_types[".ttf"] = "font/ttf";
	mime_types[".woff"] = "font/woff";
	mime_types[".woff2"] = "font/woff2";
	// Archives
	mime_types[".zip"] = "application/zip";
	mime_types[".rar"] = "application/vnd.rar";
	mime_types[".gz"] = "application/gzip";
	return (mime_types);
}

/**
 * @brief Retrieves the MIME type based on the file extension.
 *
 * This method looks up the MIME type corresponding to the file extension in the provided path.
 * If the file extension is recognized, the associated MIME type is returned. If the extension
 * is not recognized, it defaults to `text/plain`.
 *
 * @details
 * - The method extracts the file extension by searching for the last '.' character in the path.
 * - If the extension is found in the `mime_types` map, the corresponding MIME type is returned.
 * - If no recognized extension is found, the default MIME type `text/plain` is returned.
 *
 * @param path The file system path to the file.
 * @return std::string The MIME type corresponding to the file extension, or `text/plain` if not found.
 */
std::string get_mime_type(const std::string& path) {
	static const std::map<std::string, std::string> mime_types = create_mime_types();

	size_t dot_pos = path.find_last_of('.');
	if (dot_pos != std::string::npos) {
		std::string extension = path.substr(dot_pos);
		if (mime_types.find(extension) != mime_types.end()) {
			return (mime_types.at(extension));
		}
	}
	return ("text/plain");
}

/**
 * @brief Validate the MIME type based on the file extension.
 *
 * This method looks up the MIME type corresponding to the file extension in the provided path.
 * If the file extension is recognized, true is returned. If the extension is not recognized,
 * false
 *
 * @details
 * - The method extracts the file extension by searching for the last '.' character in the path.
 *
 * @param path The file system path to the file.
 * @return bool true if a MIME type is recognized, false otherwise.
 */
bool valid_mime_type(const std::string& path) {
	static const std::map<std::string, std::string> mime_types = create_mime_types();

	size_t dot_pos = path.find_last_of('.');
	if (dot_pos != std::string::npos) {
		std::string extension = path.substr(dot_pos);
		if (mime_types.find(extension) != mime_types.end()) {
			return (true);
		}
	}
	return (false);
}

bool black_list_extension(const std::string& path) {
	static std::set<std::string> disallowed_extensions;

	if (disallowed_extensions.empty()) {
		disallowed_extensions.insert(".exe");
		disallowed_extensions.insert(".bat");
		disallowed_extensions.insert(".sh");
		disallowed_extensions.insert(".php");
		disallowed_extensions.insert(".pl");
		disallowed_extensions.insert(".py");
	}

	size_t dot_pos = path.find_last_of('.');
	if (dot_pos != std::string::npos) {
		std::string extension = path.substr(dot_pos);
		if (disallowed_extensions.find(extension) != disallowed_extensions.end()) {
			return (true);
		}
	}
	return (false);
}

/**
 * @brief Replaces all occurrences of a key in the content with a given value.
 *
 * This method searches the provided content for all occurrences of the key and replaces
 * each one with the specified value. It returns the modified content with all replacements made.
 *
 * @details
 * - The method iterates through the content, finding each occurrence of the key using `std::string::find()`.
 * - For each occurrence, it replaces the key with the value using `std::string::replace()`.
 * - If the value contains the key (which could cause an infinite loop), the method does not perform any replacements.
 *
 * @param content The content in which to perform the replacements (e.g., HTML file content).
 * @param key The key to search for in the content (e.g., "{error_code}").
 * @param value The value to replace the key with (e.g., "404").
 * @return std::string The content with all occurrences of the key replaced by the value.
 */
std::string replace_template(std::string content, const std::string& key, const std::string& value) {
	size_t pos = 0;

	if (value.find(key) != std::string::npos)
		return (content);

	while ((pos = content.find(key, pos)) != std::string::npos) {
		content.replace(pos, key.length(), value);
		pos += value.length();
	}
	return (content);
}

std::string clean_host(std::string& host_to_clean) {
	std::string host = host_to_clean;
	size_t to_clean = host.find("//");
	if (to_clean != std::string::npos && host.size() > 2) {
		host = host.substr(to_clean + 2);
	}
	to_clean = host.find_last_of(':');
	if (to_clean != std::string::npos && host.size() > 1) {
		host = host.substr(0, to_clean);
	}
	to_clean = host.find('/');
	while (to_clean != std::string::npos && host.size() > 1) {
		host = host.substr(to_clean);
		to_clean = host.find('/');
	}
	return (host);
}

std::string normalize_host(const std::string& host) {
	std::string result;
	bool was_slash = false;

	for (size_t i = 0; i < host.size(); ++i) {
		if (host[i] == '/') {
			if (!was_slash) {
				result += '/';
				was_slash = true;
			}
		} else {
			result += host[i];
			was_slash = false;
		}
	}

	return (result);
}

#endif
