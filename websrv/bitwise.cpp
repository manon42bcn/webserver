/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bitwise.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/12 23:10:57 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/14 02:09:29 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <map>

// MASKS
#define MASK_METHOD_GET     (1 << 0)
#define MASK_METHOD_OPTIONS (1 << 1)
#define MASK_METHOD_HEAD    (1 << 2)
#define MASK_METHOD_POST    (1 << 3)
#define MASK_METHOD_PUT     (1 << 4)
#define MASK_METHOD_PATCH   (1 << 5)
#define MASK_METHOD_TRACE   (1 << 6)
#define MASK_METHOD_DELETE  (1 << 7)
// CHECK PRIVILEGES
#define HAS_GET(permissions)     ((permissions) & MASK_METHOD_GET)
#define HAS_OPTIONS(permissions) ((permissions) & MASK_METHOD_OPTIONS)
#define HAS_HEAD(permissions)    ((permissions) & MASK_METHOD_HEAD)
#define HAS_POST(permissions)    ((permissions) & MASK_METHOD_POST)
#define HAS_PUT(permissions)     ((permissions) & MASK_METHOD_PUT)
#define HAS_PATCH(permissions)   ((permissions) & MASK_METHOD_PATCH)
#define HAS_TRACE(permissions)   ((permissions) & MASK_METHOD_TRACE)
#define HAS_DELETE(permissions)  ((permissions) & MASK_METHOD_DELETE)
// GRANT
#define GRANT_GET(permissions)     ((permissions) |= MASK_METHOD_GET)
#define GRANT_OPTIONS(permissions) ((permissions) |= MASK_METHOD_OPTIONS)
#define GRANT_HEAD(permissions)    ((permissions) |= MASK_METHOD_HEAD)
#define GRANT_POST(permissions)    ((permissions) |= MASK_METHOD_POST)
#define GRANT_PUT(permissions)     ((permissions) |= MASK_METHOD_PUT)
#define GRANT_PATCH(permissions)   ((permissions) |= MASK_METHOD_PATCH)
#define GRANT_TRACE(permissions)   ((permissions) |= MASK_METHOD_TRACE)
#define GRANT_DELETE(permissions)  ((permissions) |= MASK_METHOD_DELETE)
// REVOKE
#define REVOKE_GET(permissions)     ((permissions) &= ~MASK_METHOD_GET)
#define REVOKE_OPTIONS(permissions) ((permissions) &= ~MASK_METHOD_OPTIONS)
#define REVOKE_HEAD(permissions)    ((permissions) &= ~MASK_METHOD_HEAD)
#define REVOKE_POST(permissions)    ((permissions) &= ~MASK_METHOD_POST)
#define REVOKE_PUT(permissions)     ((permissions) &= ~MASK_METHOD_PUT)
#define REVOKE_PATCH(permissions)   ((permissions) &= ~MASK_METHOD_PATCH)
#define REVOKE_TRACE(permissions)   ((permissions) &= ~MASK_METHOD_TRACE)
#define REVOKE_DELETE(permissions)  ((permissions) &= ~MASK_METHOD_DELETE)
// MULTI FLAGS
#define HAS_PERMISSION(permissions, mask)     ((permissions) & MASK_METHOD_GET)
#define GRANT_PERMISSIONS(permissions, mask)  ((permissions) |= (mask))
#define REVOKE_PERMISSIONS(permissions, mask) ((permissions) &= ~(mask))


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
		methods.insert(std::make_pair("PATCH" ,MASK_METHOD_PATCH));
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