/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserver.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:07:12 by mporras-          #+#    #+#             */
/*   Updated: 2024/10/14 13:51:18 by mporras-         ###   ########.fr       */
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
	if (value.empty())
		return (false);

	for (std::string::const_iterator it = value.begin(); it != value.end(); ++it) {
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
