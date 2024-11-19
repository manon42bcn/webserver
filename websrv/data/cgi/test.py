#!/usr/bin/env python3

import os
import cgi
import sys

# Configurar la codificación
sys.stdout.reconfigure(encoding='utf-8')

# Obtener el método de la solicitud
request_method = os.environ.get('REQUEST_METHOD', 'GET')

# Imprimir headers HTTP
print("Content-Type: text/html\r\n\r\n", end='')

# Procesar el formulario si es POST
form = cgi.FieldStorage()

print("""<!DOCTYPE html>
<html>
<head>
    <title>Respuesta CGI</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background-color: #f0f2f5; }
        .container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }
        .data-block { background: #f8f9fa; padding: 15px; margin: 15px 0; border-radius: 5px; }
        .success { color: #28a745; }
        .info { color: #17a2b8; }
    </style>
</head>
<body>
    <div class="container">
""")

if request_method == 'POST':
    print('<h1 class="success">Datos Recibidos!</h1>')
    print('<div class="data-block">')
    print('<h2>Datos del Formulario:</h2>')
    
    name = form.getvalue('name', 'No proporcionado')
    message = form.getvalue('message', 'No proporcionado')
    
    print(f'<p><strong>Nombre:</strong> {name}</p>')
    print(f'<p><strong>Mensaje:</strong> {message}</p>')
    print('</div>')
else:
    print('<h1 class="info">Prueba GET Exitosa</h1>')

print("""
        <div class="data-block">
            <h2>Informacion del Servidor:</h2>
            <ul>
                <li><strong>Metodo:</strong> {}</li>
                <li><strong>Python Version:</strong> {}</li>
                <li><strong>Tiempo del Servidor:</strong> {}</li>
                <li><strong>Ruta del Script:</strong> {}</li>
            </ul>
        </div>
        <p><a href="/cgi/index.html">&larr; Volver al formulario</a></p>
    </div>
</body>
</html>""".format(
    os.environ.get('REQUEST_METHOD', 'No disponible'),
    sys.version.split()[0],
    os.popen('date').read().strip(),
    os.environ.get('SCRIPT_NAME', 'No disponible'),
)) 