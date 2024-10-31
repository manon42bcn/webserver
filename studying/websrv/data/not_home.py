#!/usr/bin/env python3

import os
import cgi

print("Content-Type: text/html")
print()  # Línea en blanco para finalizar las cabeceras

# Imprimir información básica
print("<html><body>")
print("<h1>Python CGI Script - From NO_HOME.py</h1>")
print("<p>Request Method: {}</p>".format(os.environ.get("REQUEST_METHOD", "unknown")))
print("<p>Query String: {}</p>".format(os.environ.get("QUERY_STRING", "")))
print("<p>Content Type: {}</p>".format(os.environ.get("CONTENT_TYPE", "")))
print("<p>Content Length: {}</p>".format(os.environ.get("CONTENT_LENGTH", "")))
print("</body></html>")
exit()