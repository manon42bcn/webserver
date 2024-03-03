/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/03/03 13:34:12 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <iostream>
#include <fstream>

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
    return (true);
}

int main(int argc, char *argv[]) {

    if (!checkArgs(argc, argv))
        exit(1);

}