# ifndef WEBSERVER_HPP
# define WEBSERVER_HPP
#include <vector>
#include <map>

struct ServerConfig {
	int port;                            ///< Puerto en el que el servidor escuchará.
	std::string server_name;             ///< Nombre del servidor.
	std::string document_root;           ///< Directorio raíz de los archivos servidos.
	std::map<int, std::string> error_pages; ///< Páginas de error personalizadas.
	std::map<std::string, std::string> locations; ///< Mapeo de rutas a directorios o permisos.
	std::vector<std::string> default_pages; ///< Páginas predeterminadas (e.g., index.html, home.html).
};

# endif