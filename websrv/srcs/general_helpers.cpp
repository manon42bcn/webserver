/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   general_helpers.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 17:07:04 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/15 02:29:56 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "general_helpers.hpp"

bool is_dir(const std::string& path)
{
	struct stat s;

	if (stat(path.c_str(), &s) == 0) {
		return S_ISDIR(s.st_mode);
	}
	return (false);
}

bool is_file(const std::string& path) {
	struct stat s;

	if (stat(path.c_str(), &s) == 0) {
		return (S_ISREG(s.st_mode));
	}
	return (false);
}


std::string int_to_string(int number) {
	std::stringstream ss;
	ss << number;
	return (ss.str());
}

bool starts_with(const std::string& str, const std::string& prefix) {
	if (str.size() < prefix.size()) {
		return (false);
	}
	return (str.compare(0, prefix.size(), prefix) == 0);
}
