/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   general_helpers.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 17:07:04 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/15 13:55:09 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "general_helpers.hpp"

/**
 * @brief Checks if a given path is a directory.
 *
 * This function checks if the provided path corresponds to a directory by using the `stat()` system call.
 *
 * @param path The path to be checked.
 * @return `true` if the path is a directory, otherwise `false`.
 */
bool is_dir(const std::string& path)
{
	struct stat s;

	if (stat(path.c_str(), &s) == 0) {
		return S_ISDIR(s.st_mode);
	}
	return (false);
}

/**
 * @brief Checks if a given path is a regular file.
 *
 * This function checks if the provided path corresponds to a regular file by using the `stat()` system call.
 *
 * @param path The path to be checked.
 * @return `true` if the path is a regular file, otherwise `false`.
 */
bool is_file(const std::string& path) {
	struct stat s;

	if (stat(path.c_str(), &s) == 0) {
		return (S_ISREG(s.st_mode));
	}
	return (false);
}

/**
 * @brief Converts an integer to a string.
 *
 * This function converts an integer to its string representation using a stringstream.
 *
 * @param number The integer to be converted.
 * @return A string representation of the provided integer.
 */
std::string int_to_string(int number) {
	std::stringstream ss;
	ss << number;
	return (ss.str());
}

/**
 * @brief Checks if a string starts with a given prefix.
 *
 * This function checks whether the provided string begins with the specified prefix.
 *
 * @param str The string to be checked.
 * @param prefix The prefix to check for.
 * @return `true` if the string starts with the prefix, otherwise `false`.
 */
bool starts_with(const std::string& str, const std::string& prefix) {
	if (str.size() < prefix.size()) {
		return (false);
	}
	return (str.compare(0, prefix.size(), prefix) == 0);
}
