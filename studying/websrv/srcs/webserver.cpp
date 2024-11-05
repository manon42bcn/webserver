/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserver.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/28 12:39:12 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

/**
 * @brief Validates if a string represents a valid `size_t` value.
 *
 * This function checks whether the provided string contains only numeric characters (digits),
 * which would make it a valid positive integer for conversion to `size_t`.
 *
 * @details
 * - The function first checks if the string is empty. An empty string is not considered valid.
 * - It then iterates through each character of the string, verifying that all characters are digits.
 * - If all characters are digits and the string is not empty, the function returns `true`.
 * - Otherwise, it returns `false`.
 *
 * @param value The string to validate.
 * @return bool True if the string contains only digits, false otherwise.
 */
bool is_valid_size_t(const std::string& value) {
	if (value.empty()) {
		return (false);
	}

	for (std::string::const_iterator it = value.begin(); it != value.end(); ++it) {
		if (*it == ' ')
			continue ;
		if (!std::isdigit(*it))
			return (false);
	}

	return (true);
}

/**
 * @brief Converts a valid numeric string to `size_t`.
 *
 * This function converts a valid numeric string (verified externally) to a `size_t` value.
 * It assumes that the input string contains only digits, as it should be validated by `is_valid_size_t()`.
 *
 * @param value The numeric string to convert to `size_t`.
 * @return size_t The converted `size_t` value.
 */
size_t str_to_size_t(const std::string& value) {
	size_t result;
	std::stringstream ss(value);

	if (ss >> result)
		return (result);
	return (0);
}

/**
 * @brief Converts a string to lowercase.
 *
 * This function iterates through each character in the input string and converts it to lowercase
 * using `std::tolower()`. It handles characters safely by casting them to `unsigned char`.
 *
 * @param input The input string to be converted to lowercase.
 * @return std::string A new string where all characters are lowercase.
 */
std::string to_lowercase(const std::string& input) {
	std::string result = input;
	for (std::size_t i = 0; i < result.size(); ++i) {
		result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
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
std::string get_header_value(std::string& haystack, std::string needle, std::string sep) {
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


bool to_trim_char(char c, const std::string& chars_to_trim) {
	return (chars_to_trim.find(c) != std::string::npos);
}

std::string trim(const std::string& str, const std::string& chars_to_trim = " \t\n\r\f\v") {
	size_t start = 0;
	while (start < str.size() && to_trim_char(str[start], chars_to_trim)) {
		++start;
	}

	size_t end = str.size();
	while (end > start && to_trim_char(str[end - 1], chars_to_trim)) {
		--end;
	}

	return str.substr(start, end - start);
}

bool is_cgi(const std::string& filename){
	std::string cgi_files [] = {".py", ".php"};

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

//std::map<std::string, std::string> parse_headers_map(const std::string& headers) {
//	std::map<std::string, std::string> header_map;
//	std::istringstream header_stream(headers);
//	std::string line;
//
//	while (std::getline(header_stream, line)) {
//		if (!line.empty() && line.back() == '\r') {
//			line.pop_back();
//		}
//		size_t separator = line.find(": ");
//		if (separator != std::string::npos) {
//			std::string header_name = line.substr(0, separator);
//			std::string header_value = line.substr(separator + 2);
//			header_map.insert(std::make_pair(header_name, header_value));
//		}
//	}
//
//	return header_map;
//}
