#!/usr/bin/env python3

import os
import cgi

path_says = os.environ.get("PATH_INFO", "")


print("Content-Type: text/html\r\n\r\n", end='')
# print("\r\n")  # Línea en blanco para finalizar las cabeceras

# Imprimir información básica
print("<html><body>")
print(f"<h1>Python CGI Script - From not_home.py</h1>")
print("<h1>I'm force to say:</h1>")
print("<div>")
if len(path_says) > 1:
    path = path_says.split('/')
    for elem in path:
        print(f"<h3>{elem}</h3>")
print("</div>")
print("<h1>Sorry... not my fault, that was path fault..</h1>")
print("<p>Request Method: {}</p>".format(os.environ.get("REQUEST_METHOD", "unknown")))
print("<p>Query String: {}</p>".format(os.environ.get("QUERY_STRING", "")))
print("<p>Content Type: {}</p>".format(os.environ.get("CONTENT_TYPE", "")))
print("<p>Content Length: {}</p>".format(os.environ.get("CONTENT_LENGTH", "")))
print("<p>Path info: {}</p>".format(os.environ.get("PATH_INFO", "")))
print("<p>Script Name: {}</p>".format(os.environ.get("SCRIPT_NAME", "")))
print("<p>Server Name: {}</p>".format(os.environ.get("SERVER_NAME", "")))
print("<p>Server Port: {}</p>".format(os.environ.get("SERVER_PORT", "")))
print("</body></html>")