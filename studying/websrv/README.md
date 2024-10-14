# WEBSERVER

### *Básicamente aquí va una propuesta sobre lo que podría ser el formato del archivo de configuración. Lo pongo en inglés para que nos sirva de base para el README oficial*

### Configuration file

El formato final del archivo realmente no es algo que sea particularmente importante, quiero decir, si es mas cómodo poner server name {..} en lugar de server { name: "me_llamo_servidor" } es realmente irrelevante para la ejecución.
Lo que iré poniendo acá son algo así como "cosas que necesitamos" y como podríamos leerlo. Al final dejaré la estructura con los datos como deberían llegar, si alguna cosa te hace ruido cero problema, me avisas y vemos si lo movemos, cambiamos o quitamos.


* leyend: 
  - \[type\]: expected datatype.
  - \[M\]: Mandatory attribute. Error if is missing on configfile.
  - \[O\]: Optional attribute.
  - \(N\): For several options. How many can be included. (1) marked elements cannot be more than one time in its section.
  - \[enum\{..}]: One of the given options.
  - ~\[attribute\]: At least one of its marked attributes must be included at config file.
```
{
    server {
        name: (1)[M][string],
        port: (1)[M][int],
        root: (1)[M][string],
        on_error (1)[O] {
            mode: [enum: {LITERAL, TEMPLATE}],
            404: [string]...
        },
        default_pages (1)[O] ~[location/default_pages] {
                [string], [string]..
            },
        location (N)[M] {
            path: (1)[M][string],
            access: (1)[M][enum: {FORBIDDEN, READ, WRITE}],
            default_pages ~[../default_pages] {
                [string], [string]..
            },
            on_error (1)[O] {
                mode: [enum: {LITERAL, TEMPLATE}],
                404: [string]...
            },            
        }
    }
}
```
### *Details*
* server: Configures a new service, listen on a given port.
* Common properties:
  * name: server name.
  * port: listening server port (eg.8080).
  * root: host root route. Absolute route to local base path (eg: /home/etc/data/www).
  * on_error: options to configure custom error pages.
    * mode, enum:
      * LITERAL: to provide a list of custom error file for each error, using {404: 404.html, 500: 500.html...}.
      * TEMPLATE: tor provide one html template (eg: {1: template.html}). On templates two fields should be included: 
        * '{error_code}': to be replaced with html error code.
        * '{error_msg}': to be replaced with detailed message (Not Found, Forbidden, etc.).
  * default_pages: a list of default files that will be searched to serve if a path is given without details (eg: http://localhost:80/path). eg: index.html, home.html... 
* location: Configure a specific path:
  * path: eg: /, /myservice/...
  * access: enum:
    * FORBIDDEN: Any access to inner files will be rejected.
    * READ: Allows to server files from this path.
    * WRITE: Allows get, post and delete files from this path.
  * default_pages and on_error: the same configuration that server section, but for a given location. If both properties will override server configuration at location.

### Resultado de config file.

Tengo una duda en la cabeza con respecto a locations y el tema de los permisos. Ahora mismo, a partir de una ruta, podemos encontrar cualquier sub directorio.
Entiendo que locations debería funcionar igual, con lo que podríamos tener dos locations anidadas. La única cosa que podemos hacer con este tipo de anidamiento es asignar permisos específicos para cada subdirectorio que encontrmos en locations.
```
... locations {
        path: /home
    }
    locations {
        path: /home/var/foo
    }
``` 
Entonces, aquí no sé muy bien como apañarmelas, tengo varias ideas... iré informando ;-D.

Como comentamos la idea es que, antes de ejecutar, podamos contar con una cosa tipo (WIP total):

```c++
std::vector<ServerConfig> servers;
// Vector de estructuras:
struct ServerConfig {
	int                                 port;
	std::string                         server_name;
	std::string                         server_root;
	t_mode                              error_mode;
	std::map<int, std::string>          error_pages;
	std::map<std::string, std::string>  locations;
	std::vector<std::string>            default_pages;
//	------>>> General config, apply to all servers. Here to make it faster at exec
	std::string ws_root;
	std::string ws_errors_root;
	t_mode      ws_error_mode;
};
```