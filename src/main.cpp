/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/03/03 13:21:44 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

int main(int argc, char const *argv[]) {

    if (argc < 2 || argc > 3)
		std::cout << "Args error. Type --help for instructions." << std::endl;
	if (argc == 3)
		std::cout << "invalid argument" << std::endl;

    if(argv[1] == (std::string)"hi") // Testing the compare strings
        std::cout << "Hi" << argv[0] << std::endl;

}