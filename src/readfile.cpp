/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readfile.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 18:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/03/03 18:11:55 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

void readfile(char *file){

    std::string fileContent;
    char        *line;
    int         fd;
    
    fd = open(file, O_RDONLY);
    if (fd < 0) {
        std::cerr << "Error opening the file." << std::endl;
        return;
    }

    while (true) {
        line = get_next_line(fd);
        if (!line)
            break;
        std::cout << line;
        fileContent.append(line);
    }

    std::cout << fileContent;
    
    close(fd);
}