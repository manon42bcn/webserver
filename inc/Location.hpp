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

#ifndef LOCATIONS_H
#define LOCATIONS_H

#include <string>
#include <vector>
#include <iostream>
class Location {

public:

    Location();
    ~Location();

    void setPath(std::string path);
    void setRoot(std::string root);
    void setIndexFile(std::string indexFile);
    void setAutoIndex(bool autoIndex);
    void setAllowedMethods(std::vector<std::string> allowedMethods);
    void setRedirect(std::string redirect);

    std::string getPath() const;
    std::string getRoot() const;
    std::string getIndexFile() const;
    bool getAutoIndex() const;
    std::vector<std::string> getAllowedMethods() const;
    std::string getRedirect() const;


private:

    std::string path;               // Ruta del location
    std::string root;               // Directorio raíz
    std::string indexFile;          // Archivo index
    bool autoIndex;                 // Si autoindex está activado
    std::vector<std::string> allowedMethods; // Métodos HTTP permitidos
    std::string redirect;           // Redirección si existe

};

std::ostream& operator<<(std::ostream& os, const Location& location);

#endif