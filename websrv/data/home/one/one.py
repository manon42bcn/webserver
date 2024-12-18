#!/usr/bin/env python3

import time
import os
import cgi

print("Content-Type: text/html")
print()

# while True:
#     time.sleep(2)
#     print("<p>Waiting until timeout</p>")

path_says = os.environ.get("PATH_INFO", "")


print("Content-Type: text/html")
print()  # Línea en blanco para finalizar las cabeceras

# Imprimir información básica
print("<html><body>")
print("<h1>Python CGI Script - From one.py</h1>")
print("<h1>I'm force to say:</h1>")
while True:
    time.sleep(2)
    print("<p>Waiting until timeout</p>")
if len(path_says) > 1:
    path = path_says.split('/')
    for elem in path:
        print(f"<h3>{elem}</h3>")
print("<h1>Sorry... not my fault, that was path fault..</h1>")
print("<p>Request Method: {}</p>".format(os.environ.get("REQUEST_METHOD", "unknown")))
print("<p>Query String: {}</p>".format(os.environ.get("QUERY_STRING", "")))
print("<p>Content Type: {}</p>".format(os.environ.get("CONTENT_TYPE", "")))
print("<p>Content Length: {}</p>".format(os.environ.get("CONTENT_LENGTH", "")))
print("</body></html>")