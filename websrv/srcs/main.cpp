/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mporras- <manon42bcn@yahoo.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 04:30:53 by mporras-          #+#    #+#             */
/*   Updated: 2024/12/08 13:04:27 by mporras-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <map>
#include "webserver.hpp"
#include "ServerManager.hpp"
#include <signal.h>

ServerManager* running_server = NULL;

/**
 * @brief Signal handler to end webserver execution
 *
 * Due to webserver execution is an endless loop, a handler to close it properly
 * is needed. SIGINT, SIGTERM and SIGTSTP are allowed.
 *
 * @param sig Received signal.
 */
void signal_handler(int sig) {
	if (running_server != NULL) {
		std::cout << "\nReceived signal " << sig << ". Shutting down server..." << std::endl;
		running_server->turn_off_server();
	}
}

/**
 * @brief Main function - Entry point.
 *
 * @param argc count of arguments.
 * @param argv arguments of exec.
 *
 * @see ServerManager: That control all the workflow.
 */
int main(int argc, char **argv) {

	Logger logger(LOG_ERROR, false);
	std::string base_path = get_server_root();
	std::vector<ServerConfig> configs;
	std::map<std::string, LocationConfig> locations;

	if (!check_args(argc, argv))
		exit(1);
	configs = parse_file(argv[1], &logger);

	try {
		ServerManager server_manager(configs, &logger);
		signal(SIGINT, signal_handler);
		signal(SIGTERM, signal_handler);
		signal(SIGTSTP, signal_handler);
		running_server = &server_manager;
		server_manager.run();
	} catch (Logger::NoLoggerPointer& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
	} catch (WebServerException& e){
		std::cerr << "ERROR: " << e.what() << std::endl;
	} catch (std::exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
	}
	return (0);
}
