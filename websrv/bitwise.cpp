/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bitwise.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/12 23:10:57 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/13 01:55:46 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <map>
// Definición de las máscaras de bits para los métodos HTTP
# define GET_MASK (1 << 0)

#define MASK_METHOD_GET     (1 << 0)  // 0000 0001
#define MASK_METHOD_OPTIONS (1 << 1)  // 0001 0000
#define MASK_METHOD_HEAD    (1 << 2)  // 0010 0000
#define MASK_METHOD_POST    (1 << 3)  // 0000 0010
#define MASK_METHOD_PUT     (1 << 4)  // 0000 0100
#define MASK_METHOD_PATCH   (1 << 5)  // 0100 0000
#define MASK_METHOD_TRACE   (1 << 6)  // 1000 0000
#define MASK_METHOD_DELETE  (1 << 7)  // 0000 1000
#define mask_read (MASK_METHOD_GET | MASK_METHOD_HEAD | MASK_METHOD_OPTIONS)
#define mask_read_write  (MASK_METHOD_GET | MASK_METHOD_HEAD | MASK_METHOD_OPTIONS | MASK_METHOD_POST)

unsigned char method_bitwise(std::string parsed) {
	static std::map<std::string, unsigned char> methods;
	if (methods.empty()) {
		methods.insert(std::make_pair("GET", MASK_METHOD_GET));
		methods.insert(std::make_pair("OPTIONS",MASK_METHOD_OPTIONS));
		methods.insert(std::make_pair("HEAD" ,MASK_METHOD_HEAD));
		methods.insert(std::make_pair("POST",MASK_METHOD_POST));
		methods.insert(std::make_pair("PUT",MASK_METHOD_PUT));
		methods.insert(std::make_pair("PATHC" ,MASK_METHOD_PATCH));
		methods.insert(std::make_pair("TRACE",MASK_METHOD_TRACE));
		methods.insert(std::make_pair("DELETE",MASK_METHOD_DELETE));
	}
	std::map<std::string, unsigned char>::const_iterator it = methods.find(parsed);
	if (it != methods.end()) {
		return (it->second);
	}
	return (0);
}

#include <ostream>
#include <string>
#include <sstream>



int main() {
	unsigned char permissions = 0;  // Ningún método permitido inicialmente
	// Otorgar permisos
	permissions |= method_bitwise("GET");
	permissions |= method_bitwise("POST");

	// Verificar permisos
	if (permissions & MASK_METHOD_GET) {
		std::cout << "GET permitido" << std::endl;
	}

	if (permissions & MASK_METHOD_DELETE) {
		std::cout << "DELETE permitido" << std::endl;
	} else {
		std::cout << "DELETE no permitido" << std::endl;
	}

	// Revocar  permisos
	permissions &= ~method_bitwise("POST");

	if (permissions & MASK_METHOD_POST) {
		std::cout << "POST nos vale" << std::endl;
	} else {
		std::cout << "POST se fue" << std::endl;
	}

	unsigned char multiple = 0;
	// estos tres equivalen a permisos de lectura.
	multiple |= MASK_METHOD_GET;
	multiple |= MASK_METHOD_HEAD;
	multiple |= MASK_METHOD_OPTIONS;

	if ((multiple & mask_read) == mask_read) {
		std::cout << "Permisos de lectura" << std::endl;
	} else {
		std::cout << "Get out of here becerro" << std::endl;
	}

	if ((permissions & MASK_METHOD_DELETE) == 0) {
		std::cout << "DELETE nanai." << std::endl;
	} else {
		std::cout << "DELETE esta aqui.." << std::endl;
	}
	multiple |= MASK_METHOD_POST;
//	multiple = 0;
//	// estos tres equivalen a permisos de lectura escritura, no delete.
//	multiple |= METHOD_GET;
//	multiple |= METHOD_HEAD;
//	multiple |= METHOD_OPTIONS;

	if ((multiple & mask_read_write) == mask_read_write) {
		std::cout << "Permisos de lectura - escritura" << std::endl;
	} else {
		std::cout << "Get out of here becerro" << std::endl;
	}
	return 0;
}