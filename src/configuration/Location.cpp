/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/03 18:03:40 by vaguilar          #+#    #+#             */
/*   Updated: 2024/03/23 18:00:22 by vaguilar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"
#include "Config.hpp"

Location::Location() {
    this->path = "";
    this->root = "";
    this->indexFile = "";
    this->autoIndex = false;
    this->allowedMethods = std::vector<std::string>();
    this->redirect = "";
}

Location::~Location() { }

void Location::setPath(std::string path) {
    std::string pathType; // Not used

    if (path[0] != '/' && isspace(path[1]))
    {
        std::string::size_type spacePos = path.find_first_of(" ");
        if (spacePos != std::string::npos) {
            pathType = path.substr(0, spacePos);
            path = path.substr(spacePos + 1);
        } else {
            pathType = "=";
        }
    }

    std::string::size_type bracketPos = path.find("{");
    if (bracketPos != std::string::npos) {
        path = path.substr(0, bracketPos);
    }

    path.erase(0, path.find_first_not_of(" \t"));
    path.erase(path.find_last_not_of(" \t") + 1);


    this->path = path;
}

void Location::setRoot(std::string root) {
    this->root = root;
}

void Location::setIndexFile(std::string indexFile) {
    this->indexFile = indexFile;
}

void Location::setAutoIndex(bool autoIndex) {
    this->autoIndex = autoIndex;
}

void Location::setAllowedMethods(std::string allowedMethods) {
    std::vector<std::string> methods;
    std::istringstream iss(allowedMethods);
    std::string method;
    while (iss >> method) {
        methods.push_back(method);
    }

    this->allowedMethods = methods;
}

void Location::setRedirect(std::string redirect) {
    this->redirect = redirect;
}

std::string Location::getPath() const {
    return this->path;
}

std::string Location::getRoot() const {
    return this->root;
}

std::string Location::getIndexFile() const {
    return this->indexFile;
}

bool Location::getAutoIndex() const {
    return this->autoIndex;
}

std::vector<std::string> Location::getAllowedMethods() const {
    return this->allowedMethods;
}

std::string Location::getRedirect() const {
    return this->redirect;
}

std::ostream& operator<<(std::ostream& os, const Location& location) {
    os << GRAY << "Path: " << DEF_COLOR << location.getPath() << std::endl;
    os << GRAY << "Root: " << DEF_COLOR << location.getRoot() << std::endl;
    os << GRAY << "Index: " << DEF_COLOR << location.getIndexFile() << std::endl;
    if (!location.getIndexFile().empty())
        os << GRAY << "AutoIndex: " << DEF_COLOR << "true" << std::endl;
    if (location.getAutoIndex() == false)
        os << GRAY << "AutoIndex: " << DEF_COLOR << "false" << std::endl;
    os << GRAY << "Allowed method: " << DEF_COLOR;
    const std::vector<std::string>& allowedMethods = location.getAllowedMethods();    for (std::vector<std::string>::const_iterator it = allowedMethods.begin(); it != allowedMethods.end(); it++) {
        os << *it << " ";
    }
    std::endl(os);

    // os << GRAY << "Redirect: " << DEF_COLOR << location.getRedirect() << std::endl;
    return os;
}
