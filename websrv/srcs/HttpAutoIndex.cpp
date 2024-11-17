/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpAutoIndex.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/17 22:09:10 by mporras-          #+#    #+#             */
/*   Updated: 2024/11/18 00:39:52 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpAutoIndex.hpp"

HttpAutoIndex::HttpAutoIndex(const LocationConfig *location,
							 const Logger *log,
							 ClientData *client_data,
							 s_request &request,
							 int fd):
							 WsResponseHandler(location, log, client_data, request, fd) {
	_log->log_debug( AI_NAME,
			  "Auto Index Handler init.");
}

bool HttpAutoIndex::handle_request() {
	if (!HAS_GET(_location->loc_allowed_methods)) {
		turn_off_sanity(HTTP_FORBIDDEN,
		                "Get not allowed over resource.");
		send_error_response();
		return (false);
	}
	get_file_content(_request.normalized_path);
	if (_response_data.status) {
		_response_data.mime = "text/html";
		_log->log_debug( RSP_NAME,
		                 "File content will be sent.");
		return (send_response(_response_data.content, _request.normalized_path));
	}
	_log->log_debug( RSP_NAME,
	                 "Get will send a error due to content load fails.");
	return (send_error_response());
}

void HttpAutoIndex::get_file_content(std::string& path) {
	try {
		DIR* dir = opendir(path.c_str());
		if (dir == NULL) {
			turn_off_sanity(HTTP_NOT_FOUND,
			                "Dir cannot be open.");
			return ;
		}
		std::string path_to_file = _request.path;
		if (path_to_file[path_to_file.size() - 1] != '/') {
			path_to_file += "/";
		}
		std::ostringstream out;
		out << "<!DOCTYPE html>\n<html>\n<head>\n<title>Index of " << _request.path << "</title>\n";
		out << "<style>" << AUTOINDEX_STYLE << FOOTER_STYLE << "</style>\n";
		out << "</head>\n<body>\n";
		out << "<h1>AutoIndex of : " << _request.path << "</h1>\n";
		out << "<table>\n<tr><th>Name</th><th>Size</th><th>Last Modified</th></tr>\n";

		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL) {
			std::string name = entry->d_name;
			if (name == "." || name == "..") {
				continue;
			}
			std::string base_path = _request.normalized_path + name;
			struct stat file_stat;
			if (stat(base_path.c_str(), &file_stat) == -1) {
				_log->log_error(AI_NAME,
				                "Error reading file.");
				continue;
			}
			out << "<tr>";
			out << "<td><a href=\"" << path_to_file << name << "\">" << name << "</a></td>";
			if (S_ISDIR(file_stat.st_mode)) {
				out << "<td>Directory</td>";
			} else {
				std::ostringstream size_stream;
				size_stream << file_stat.st_size;
				out << "<td>" << size_stream.str() << " bytes</td>";
			}
			char time_buf[80];
			std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", std::localtime(&file_stat.st_mtime));
			out << "<td>" << time_buf << "</td>";

			out << "</tr>\n";
		}

		out << "</table>\n" << FOOTER_GENERAL << "</body>\n</html>";
		closedir(dir);
		_response_data.content = out.str();
		_response_data.status = true;
	} catch (std::exception& e) {
		std::ostringstream detail;
		detail << "Error Creating Autoindex file for " << path << ". Error: " << e.what();
		_log->log_warning(AI_NAME,detail.str());
		turn_off_sanity(HTTP_INTERNAL_SERVER_ERROR,
						"Error Creating Autoindex data.");
		_response_data.status = false;
	}
}

void HttpAutoIndex::get_file_content(int pid, int (&fd)[2]) {
	UNUSED(pid);
	UNUSED(fd);
}
