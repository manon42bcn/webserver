/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserver.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 18:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/03/11 19:59:03 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERVER_H
# define WEBSERVER_H

#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include "ConfigurationFile.hpp"
extern "C" {
#include "libft.h"
}

void readfile(char *file);

#endif