#!/usr/bin/env python3

import os
import sys
import cgi
import cgitb
import io

# Activar el debug de CGI
cgitb.enable(display=0, logdir="/tmp")

# Directorio donde se guardarán los archivos
UPLOAD_DIR = os.path.join(os.path.dirname(__file__), 'upload')

def save_file(post_data):
    try:
        # Extraer el boundary de los datos
        first_line = post_data.split(b'\r\n')[0]
        boundary = first_line.decode()

        # Crear un archivo temporal con los datos
        temp_file = io.BytesIO(post_data)

        # Configurar el environment para el FieldStorage
        environ = os.environ.copy()
        environ['CONTENT_TYPE'] = f'multipart/form-data; boundary={boundary[2:]}'

        # Procesar el formulario
        form = cgi.FieldStorage(
            fp=temp_file,
            environ=environ,
            keep_blank_values=True
        )

        if "file" not in form:
            return False, "No se recibió ningún archivo"

        fileitem = form["file"]
        if not fileitem.filename:
            return False, "No se seleccionó ningún archivo"

        # Obtener el nombre del archivo de forma segura
        filename = os.path.basename(fileitem.filename)
        filepath = os.path.join(UPLOAD_DIR, filename)

        # Guardar el archivo
        with open(filepath, 'wb') as f:
            if hasattr(fileitem.file, 'read'):
                while True:
                    chunk = fileitem.file.read(8192)
                    if not chunk:
                        break
                    f.write(chunk)
            else:
                f.write(fileitem.value)

        description = form.getvalue("description", "")
        return True, (filename, description)
    except Exception as e:
        return False, f"Error al procesar el archivo: {str(e)}"

try:
    # Asegurar que el directorio de uploads existe
    if not os.path.exists(UPLOAD_DIR):
        os.makedirs(UPLOAD_DIR, mode=0o755)

    # Verificar el método HTTP
    if os.environ.get('REQUEST_METHOD') != 'POST':
        raise Exception("Método no permitido. Use POST para subir archivos.")

    # Leer los datos directamente del stdin
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    post_data = sys.stdin.buffer.read(content_length)

    # Guardar los datos crudos para debug
    debug_file = os.path.join(os.path.dirname(__file__), 'raw_post_data.txt')
    with open(debug_file, 'wb') as f:
        f.write(post_data)

    # Imprimir headers HTTP
    sys.stdout.write("Content-Type: text/html\n\n")

    # Intentar guardar el archivo
    success, result = save_file(post_data)

    print(f"""<!DOCTYPE html>
    <html>
    <head>
        <title>Resultado de la Subida</title>
        <style>
            body {{ font-family: Arial, sans-serif; margin: 40px; }}
            pre {{ background: #f5f5f5; padding: 15px; border-radius: 5px; overflow-x: auto; }}
            .success {{ color: #28a745; }}
            .error {{ color: #dc3545; }}
        </style>
    </head>
    <body>
        <h2>Debug Information:</h2>
        <pre>
Content-Length: {content_length}
Bytes leídos: {len(post_data)}

Primeros 1024 bytes:
{post_data[:1024]}
        </pre>

        <h2>Resultado:</h2>""")

    if success:
        filename, description = result
        print(f"""
        <div class="success">
            <h3>¡Archivo Subido Exitosamente!</h3>
            <p>Archivo: {filename}</p>
            <p>Descripción: {description}</p>
        </div>
        """)
    else:
        print(f"""
        <div class="error">
            <h3>Error al subir el archivo</h3>
            <p>{result}</p>
        </div>
        """)

    print("""
        <p><a href="/cgi/update.html">&larr; Volver al formulario</a></p>
    </body>
    </html>""")

except Exception as e:
    sys.stdout.write("Content-Type: text/html\n\n")
    print(f"""<!DOCTYPE html>
    <html>
    <head>
        <title>Error</title>
        <style>
            body {{ font-family: Arial, sans-serif; margin: 40px; }}
            .error {{ color: #dc3545; }}
        </style>
    </head>
    <body>
        <h1 class="error">Error del Servidor</h1>
        <p>{str(e)}</p>
        <p><a href="/cgi/update.html">&larr; Volver al formulario</a></p>
    </body>
    </html>
    """)