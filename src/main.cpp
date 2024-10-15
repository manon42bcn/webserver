/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 13:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/10/15 22:47:39 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserver.hpp"

int main(int argc, char *argv[]) {
    if (!check_args(argc, argv))
        exit(1);
    parse_file(argv[1]);

    exit(0);
}
