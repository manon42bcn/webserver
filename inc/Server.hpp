/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 18:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/03/23 17:59:53 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <vector>
#include <map>
#include <string>

class Server {

public:

    Server();
    ~Server();

    void setHost(std::string host);
    void setPort(std::string port);
    void setServerName(std::string server_name);
    void setErrorPage(std::string error_page);
    void setClientMaxBodySize(std::string client_max_body_size);
    void setAutoindex(std::string autoindex);

    std::string getHost() const;
    std::string getPort() const;
    std::string getServerName() const;
    std::string getErrorPage() const;
    std::string getClientMaxBodySize() const;
    std::string getAutoindex() const;

    bool checkObligatoryParams();

    void throwError(const std::string& errorMessage);

private:

    std::string _host;
    std::string _port;
    std::string _server_name;
    std::string _error_page;
    std::string _client_max_body_size;
    std::string _autoindex;

};

std::ostream& operator<<(std::ostream& os, const Server& server);

#endif
