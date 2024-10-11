# ifndef WEBSERVER_HPP
# define WEBSERVER_HPP
#include <vector>
#include <map>
#include <iostream>
#include <sstream>

std::string int_to_string(int number);
struct ServerConfig {
	int port;                            ///< Puerto en el que el servidor escuchará.
	std::string server_name;             ///< Nombre del servidor.
	std::string document_root;           ///< Directorio raíz de los archivos servidos.
	std::map<int, std::string> error_pages; ///< Páginas de error personalizadas.
	std::map<std::string, std::string> locations; ///< Mapeo de rutas a directorios o permisos.
	std::vector<std::string> default_pages; ///< Páginas predeterminadas (e.g., index.html, home.html).
};
void print_server_config(const ServerConfig& config, std::string location);
void print_vector_config(const std::vector<ServerConfig> &config, std::string location);

//void print_server_config(const ServerConfig& config) {
//	std::cout << "Server Configuration:" << std::endl;
//	std::cout << "  Port: " << config.port << std::endl;
//	std::cout << "  Server Name: " << config.server_name << std::endl;
//	std::cout << "  Document Root: " << config.document_root << std::endl;
//
//	// Imprimir las páginas de error personalizadas
//	std::cout << "  Error Pages: " << std::endl;
//	for (std::map<int, std::string>::const_iterator it = config.error_pages.begin(); it != config.error_pages.end(); ++it) {
//		std::cout << "    Error " << it->first << ": " << it->second << std::endl;
//	}
//
//	// Imprimir las páginas por defecto
//	std::cout << "  Default Pages: " << std::endl;
//	for (std::vector<std::string>::const_iterator it = config.default_pages.begin(); it != config.default_pages.end(); ++it) {
//		std::cout << "    " << *it << std::endl;
//	}
//
//	// Imprimir las rutas configuradas en 'locations'
//	std::cout << "  Locations: " << std::endl;
//	for (std::map<std::string, std::string>::const_iterator it = config.locations.begin(); it != config.locations.end(); ++it) {
//		std::cout << "    Path: " << it->first << ", Directory: " << it->second << std::endl;
//	}
//
//	std::cout << std::endl;
//}
//
//void print_vector_config(std::vector<ServerConfig> &config)
//{
//	for (size_t i = 0; i < config.size(); i++)
//		print_server_config(config[i]);
//}

# endif