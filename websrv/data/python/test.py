#!/usr/bin/env python3

import os
import cgi

path_says = os.environ.get("PATH_INFO", "")

print("Content-Type: text/html")
print()  # Línea en blanco para finalizar las cabeceras

print("<html><body>")
print("<h1>Python CGI Test Script</h1>")
print("<p>This is a test CGI response</p>")
print("<p>Path Info: {}</p>".format(os.environ.get("PATH_INFO", "")))
print("<p>Request Method: {}</p>".format(os.environ.get("REQUEST_METHOD", "unknown")))
print("<p>Query String: {}</p>".format(os.environ.get("QUERY_STRING", "")))
print("</body></html>") 