#include "HttpRequestHandler.hpp"
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>


std::map<std::string, std::string> HttpRequestHandler::create_mime_types() {
	std::map<std::string, std::string> mime_types;
	mime_types[".html"] = "text/html";
	mime_types[".css"] = "text/css";
	mime_types[".js"] = "application/javascript";
	mime_types[".jpg"] = "image/jpeg";
	mime_types[".jpeg"] = "image/jpeg";
	mime_types[".png"] = "image/png";
	mime_types[".gif"] = "image/gif";
	mime_types[".json"] = "application/json";
	mime_types[".txt"] = "text/plain";
	return mime_types;
}

std::string HttpRequestHandler::get_mime_type(const std::string& path) {
	static const std::map<std::string, std::string> mime_types = create_mime_types();

	size_t dot_pos = path.find_last_of('.');
	if (dot_pos != std::string::npos) {
		std::string extension = path.substr(dot_pos);
		if (mime_types.find(extension) != mime_types.end()) {
			return (mime_types.at(extension));
		}
	}
	return ("text/plain");
}

/**
 * @brief Read the request from the socket.
 *
 * @param client_socket Client FD
 * @return std::string Full request string format.
 */
std::string HttpRequestHandler::read_http_request(int client_socket) {
	char buffer[1024];
	std::string request;
	int valread;

	while ((valread = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
		buffer[valread] = '\0';
		request += buffer;
		if (request.find("\r\n\r\n") != std::string::npos) {
			break;
		}
	}

	return (request);
}

/**
 * @brief Parse requested route
 *
 * @param request Request string format
 * @return std::string Requested route
 */
std::string HttpRequestHandler::parse_requested_path(const std::string& request) {
	size_t pos = request.find(" ");
	if (pos != std::string::npos) {
		size_t end_pos = request.find(" ", pos + 1);
		if (end_pos != std::string::npos) {
			return request.substr(pos + 1, end_pos - pos - 1);
		}
	}
	return "/";
}

/**
 * @brief HTTP Request handler
 *
 * @param client_socket Client FD.
 * @param config Server config struct
 */
void HttpRequestHandler::handle_request(int client_socket, const ServerConfig config) {
	std::string request = read_http_request(client_socket);
	std::string requested_path = parse_requested_path(request);

	// Manejo de la solicitud para el root "/"
	if (requested_path == "/") {
		for (size_t i = 0; i < config.default_pages.size(); ++i) {
			std::string full_path = config.document_root + "/" + config.default_pages[i];
			std::ifstream file(full_path.c_str(), std::ios::binary);
			if (file.is_open()) {
				// Leer y enviar el archivo encontrado
				std::stringstream file_content;
				file_content << file.rdbuf();
				std::string content = file_content.str();

				std::string response = response_header(200, "OK", content.length(), get_mime_type(full_path));
				response += content;
				send(client_socket, response.c_str(), response.length(), 0);
				file.close();
				return;
			}
		}
	}

	// Manejo de solicitudes a subdirectorios "/casos/"
	if (requested_path.back() == '/') {
		for (size_t i = 0; i < config.default_pages.size(); ++i) {
			std::string full_path = config.document_root + requested_path + config.default_pages[i];
			std::ifstream file(full_path.c_str(), std::ios::binary);
			if (file.is_open()) {
				// Leer y enviar el archivo encontrado
				std::stringstream file_content;
				file_content << file.rdbuf();
				std::string content = file_content.str();

				std::string response = response_header(200, "OK", content.length(), get_mime_type(full_path));
				response += content;
				send(client_socket, response.c_str(), response.length(), 0);
				file.close();
				return;
			}
		}
	}

	// Manejo de solicitudes a archivos específicos
	std::string full_path = config.document_root + requested_path;
	std::ifstream file(full_path.c_str(), std::ios::binary);

	if (file.is_open()) {
		// Leer y enviar el archivo solicitado
		std::stringstream file_content;
		file_content << file.rdbuf();
		std::string content = file_content.str();

		std::string response = response_header(200, "OK", content.length(), get_mime_type(full_path));
		response += content;

		send(client_socket, response.c_str(), response.length(), 0);
		file.close();
	} else {
		send_error_response(client_socket, config, 404);
	}
}


/**
 * @brief Send an error page to the client
 *
 * If the error page does not exist, it will send a text to avoid server silent
 *
 * @param client_fd client FD
 * @param config ServerConfig struct
 * @param error_code Error code.
 */
void HttpRequestHandler::send_error_response(int client_fd, const ServerConfig config, int error_code) {
	std::string error_page_path;
	bool file_found = false;

	// Intentar obtener la página de error del archivo
	if (config.error_pages.find(error_code) != config.error_pages.end()) {
		error_page_path = config.document_root + config.error_pages.at(error_code);
		std::ifstream file(error_page_path.c_str(), std::ios::binary);
		if (file.is_open()) {
			// Leer y enviar el archivo de error
			std::stringstream file_content;
			file_content << file.rdbuf();
			std::string content = file_content.str();

			std::string response = response_header(error_code, "Error", content.length(), "text/plain");
			response += content;

			send(client_fd, response.c_str(), response.length(), 0);
			file.close();
			file_found = true;
		}
	}

	// Si no se encontró el archivo, enviar un texto predeterminado
	if (!file_found) {
		std::string default_text = "Error " + int_to_string(error_code) + " - Error page not found.";
		std::string response = response_header(error_code, "Error", default_text.length(), "text/plain");
		response += default_text;

		send(client_fd, response.c_str(), response.length(), 0);
	}
}

/**
 * @brief Creates the normal header to include at response
 *
 * @param code HTML status code to include (200, 404, etc)
 * @param result Verbose details to include with status code (OK, Error..)
 * @param content_size len of content to include at response
 */
std::string HttpRequestHandler::response_header(int code,
												std::string result,
												size_t content_size,
												std::string mime)
{
	std::string header = "HTTP/1.1 " + int_to_string(code) + " " + result + "\r\n";
	header += "Content-Length: " + int_to_string(content_size) + "\r\n";
	header += "Content-Type: \"" + mime + "\"\r\n";
	header += "\r\n";
	return (header);
}