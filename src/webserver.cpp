/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserver.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/03/03 17:28:33 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>


extern "C" {
#include "libft.h"
}


bool checkArgs(int argc, char **argv)
{
    std::ifstream file;
    file.open(argv[1]);

    if (argc < 2)
    {
        std::cout << "Args error. Type --help for instructions." << std::endl;
        return (false);
    }
    if (!file)
    {
        std::cout << "Error: file \"" << argv[1] << "\" cant be opened." << std::endl;
        return (false);
    }
    //Add other errors in file, access...

    return (true);
}

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

int main(int argc, char *argv[]) {

    if (!checkArgs(argc, argv))
        exit(1);
    readfile(argv[1]);

}