#!/usr/bin/env python3

import os

# Imprimir headers HTTP (importante el \r\n\r\n)
print("Content-Type: text/html\r\n\r\n", end='')

# Contenido HTML simple
print("""<!DOCTYPE html>
<html>
<head>
    <title>Simple CGI Test</title>
</head>
<body>
    <h1>CGI Test Working</h1>
    <p>Method: {}</p>
    <p>Path: {}</p>
</body>
</html>""".format(
    os.environ.get('REQUEST_METHOD', 'Not set'),
    os.environ.get('PATH_INFO', 'Not set')
)) 