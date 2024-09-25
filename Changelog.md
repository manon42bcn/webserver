# WEBSERVER CHANGELOG

- studying: Directorio para compartir mi primera aproximación medianamente seria. Sin parseo, únicamente un server que hace echo "Hello World!"
- Directorio: 
  - studying/
    ├── Makefile
    └── websrv
    ├── Dockerfile
    ├── Makefile
    ├── docker-compose.yml
    ├── inc
    │   └── webserver.hpp
    └── srcs
    └── main.cpp
- Dockerizado el Makefile de studying/Makefile lo que hace es que monta el contenedor, copia los archivos y compila, dentro del contenedor, el webserver. Lo inicia automáticamente.
- Ya montado, se pueden hacer peticiones desde fuera del contendor a localhost:8080.
---
