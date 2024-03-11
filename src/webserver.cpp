/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserver.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/03/11 20:22:20 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"
#include "ConfigurationFile.hpp"

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

int main(int argc, char *argv[]) {

    if (!checkArgs(argc, argv))
        exit(1);
    ConfigFile configFile(argv[1]);

    std::cout << configFile.getValue("ping") << std::endl;
    // readfile(argv[1]);

}