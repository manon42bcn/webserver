/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_request_helper.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 22:49:58 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/23 23:39:14 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http_enum_codes.hpp"
#include <map>
#include <iostream>
#include <set>

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

/**
 * @brief Parses an HTTP method string and returns the corresponding method mask.
 *
 * This function maps a given HTTP method string (e.g., "GET", "POST") to its corresponding
 * predefined method mask (`t_methods`). If the method is not recognized, it returns 0.
 *
 * @param method A string representing the HTTP method to parse (e.g., "GET", "POST").
 * @return The corresponding `t_methods` mask if the method is recognized; otherwise, returns 0.
 *
 * @details
 * - The method uses a static map to store the mapping between HTTP method strings
 *   and their corresponding bitmask constants (e.g., `MASK_METHOD_GET` for "GET").
 * - The map is initialized only once, ensuring efficiency for subsequent calls.
 * - If the provided method string is not found in the map, the function returns 0,
 *   indicating an unrecognized or unsupported method.
 *
 * @note
 * - Supported methods include: "GET", "POST", "DELETE", "PUT", "HEAD", "OPTIONS", and "PATCH".
 * - This function is case-sensitive. Ensure that the input method string matches the
 *   expected format (e.g., "GET" must be uppercase).
 */
t_methods parse_method(const std::string& method)
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

/**
 * @brief Checks if a given file path has a disallowed extension.
 *
 * This function determines whether the file extension of the provided path
 * is part of a predefined blacklist of disallowed extensions. If the extension
 * is blacklisted, the function returns `true`; otherwise, it returns `false`.
 *
 * @param path A string representing the file path to check.
 * @return `true` if the file's extension is blacklisted; otherwise, `false`.
 *
 * @details
 * - The function uses a static set of disallowed extensions, which is initialized
 *   on the first call. The blacklist includes extensions such as `.exe`, `.bat`, `.sh`, `.php`, `.pl`, and `.py`.
 * - The function extracts the file extension by locating the last `.` character
 *   in the file path and comparing the substring to the blacklist.
 * - If no `.` character is found in the path, or if the extension is not in the
 *   blacklist, the function returns `false`.
 */
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

/**
 * @brief Cleans up a host string by removing protocol prefixes, port numbers, and path segments.
 *
 * This function processes a host string to extract the clean hostname by:
 * 1. Removing any leading protocol prefixes (e.g., "http://").
 * 2. Stripping port numbers appended with a colon (e.g., ":8080").
 * 3. Removing any path segments following the hostname (e.g., "/path/to/resource").
 *
 * @param host_to_clean A reference to the original host string to be cleaned.
 * @return A cleaned host string containing only the hostname without prefixes, ports, or paths.
 *
 * @details
 * - The function works step by step:
 *   1. Finds and removes the `//` sequence, which typically appears in protocol prefixes like "http://".
 *   2. Removes anything after the last `:` to discard port numbers.
 *   3. Iteratively removes any leading `/` characters and paths.
 * - If the input string does not contain these patterns, it is returned as-is.
 *
 * @note
 * - This function assumes the input string follows a valid URL-like format.
 * - The function performs string operations such as `find` and `substr`, which are safe in C++98.
 */
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

/**
 * @brief Normalizes a host string by collapsing consecutive slashes into a single slash.
 *
 * This function processes a host string and ensures that sequences of consecutive slashes
 * (`/`) are reduced to a single slash. The resulting string maintains the original order
 * of characters but removes unnecessary redundancy caused by multiple slashes.
 *
 * @param host A string representing the host to normalize.
 * @return A normalized string where consecutive slashes are collapsed into a single slash.
 *
 * @details
 * - The function iterates through each character in the input string, appending characters
 *   to the result while collapsing sequences of `/` into a single instance.
 * - The logic uses a `was_slash` flag to track whether the last character processed was a slash,
 *   ensuring that additional slashes are ignored.
 * - If the input string does not contain redundant slashes, it is returned unchanged.
 *
 * @example
 * std::string raw_host = "example.com////path//to//resource";
 * std::string normalized_host = normalize_host(raw_host);
 * // normalized_host will be "example.com/path/to/resource"
 */
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

/**
 * @brief Extracts the value of a specific HTTP header field.
 *
 * This method searches the provided header string for a specific key and returns the associated
 * value. The search is case-insensitive, and it assumes the format `key: value`.
 *
 * @details
 * - The method first converts the key and the header string to lowercase for a case-insensitive search.
 * - The value is extracted by searching for the next occurrence of `\r\n`, which signifies the end of the value.
 * - If the key is not found, the method returns an empty string.
 *
 * @param haystack The HTTP Header format string to be searched over it.
 * @param needle The key for which the value is to be retrieved (e.g., "content-type").
 * @return std::string The value associated with the key, or an empty string if the key is not found.
 */
std::string get_header_value(std::string& haystack, std::string needle, const std::string& sep) {
	std::string lower_header = to_lowercase(haystack);
	size_t key_pos = lower_header.find(needle);

	if (key_pos != std::string::npos) {
		key_pos += needle.length() + 1;
		size_t end_key = lower_header.find(sep, key_pos);
		if (end_key == std::string::npos) {
			return (haystack.substr(key_pos));
		} else {
			return (haystack.substr(key_pos, end_key - key_pos));
		}
	}
	return ("");
}

/**
 * @brief Checks if a file is a CGI script based on its extension.
 *
 * This function checks whether the given filename corresponds to a CGI script by looking at its extension.
 * It checks for extensions such as `.py` and `.php`.
 *
 * @param filename The filename to be checked.
 * @return `true` if the filename corresponds to a CGI script, otherwise `false`.
 */
bool is_cgi(const std::string& filename){
	std::string cgi_files [] = {".py", ".pl"};

	size_t dot_pos = filename.find_last_of('.');
	if (dot_pos == std::string::npos) {
		return (false);
	}
	std::string extension = filename.substr(dot_pos);
	for (size_t i = 0; i < cgi_files[i].size() ; i++) {
		if (extension == cgi_files[i]) {
			return (true);
		}
	}
	return (false);
}

/**
 * @brief Finds the end of the HTTP header in a string.
 *
 * This function locates the end of the HTTP header in the given string by searching for the sequence `\\r\\n\\r\\n` or `\\n\\n`.
 *
 * @param header The HTTP header string to be analyzed.
 * @return The position where the header ends. If no header end is found, returns `std::string::npos`.
 */
size_t end_of_header_system(std::string& header)
{
	size_t  pos = header.find("\r\n\r\n");
	if (pos == std::string::npos) {
		pos = header.find("\n\n");
	}
	return (pos);
}

