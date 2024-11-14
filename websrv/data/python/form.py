#!/usr/bin/env python3

import os
import cgi
import cgitb

# Activar el debug de CGI
cgitb.enable()

# Obtener el método de la solicitud
request_method = os.environ.get('REQUEST_METHOD', 'GET')

# Imprimir los headers necesarios
print("Content-Type: text/html\r\n\r\n", end='')

# HTML inicial
print("""<!DOCTYPE html>
<html>
<head>
    <title>Formulario CGI Python</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        form { background: #f0f0f0; padding: 20px; border-radius: 8px; }
        input[type="text"] { padding: 8px; margin: 10px 0; width: 200px; }
        input[type="submit"] { background: #4CAF50; color: white; padding: 10px 15px; border: none; border-radius: 4px; }
    </style>
</head>
<body>
    <h1>Formulario de Prueba CGI</h1>
""")

# Si es POST, procesar el formulario
if request_method == 'POST':
    form = cgi.FieldStorage()
    nombre = form.getvalue('nombre', 'No especificado')
    mensaje = form.getvalue('mensaje', 'No especificado')
    
    print(f"""
    <h2>Datos Recibidos:</h2>
    <p><strong>Nombre:</strong> {nombre}</p>
    <p><strong>Mensaje:</strong> {mensaje}</p>
    <hr>
    """)

# Mostrar el formulario
print("""
    <form method="POST" action="/python/form.py">
        <p>
            <label for="nombre">Nombre:</label><br>
            <input type="text" name="nombre" id="nombre" required>
        </p>
        <p>
            <label for="mensaje">Mensaje:</label><br>
            <input type="text" name="mensaje" id="mensaje" required>
        </p>
        <input type="submit" value="Enviar">
    </form>

    <h2>Información del Entorno:</h2>
    <ul>
        <li>Método: {}</li>
        <li>Query String: {}</li>
        <li>Content Length: {}</li>
        <li>Content Type: {}</li>
    </ul>
</body>
</html>
""".format(
    os.environ.get('REQUEST_METHOD', 'No disponible'),
    os.environ.get('QUERY_STRING', 'No disponible'),
    os.environ.get('CONTENT_LENGTH', 'No disponible'),
    os.environ.get('CONTENT_TYPE', 'No disponible')
)) 