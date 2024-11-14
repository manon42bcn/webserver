/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   general_helpers.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 17:08:01 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/14 17:15:09 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GENERAL_HELPERS_HPP
#define GENERAL_HELPERS_HPP

#include <sstream>
#include <sys/stat.h>
#include <cstring>

bool is_dir(const std::string& path);
bool is_file(const std::string& path);
std::string int_to_string(int number);
bool starts_with(const std::string& str, const std::string& prefix);

#endif
