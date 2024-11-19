#!/usr/bin/env python3
import os
import cgi
import cgitb

cgitb.enable()

print("Content-Type: text/html")
print()

try:
    # Obtener la ruta absoluta del directorio donde está el script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    upload_dir = os.path.join(script_dir, "upload")

    form = cgi.FieldStorage()
    filename = form.getvalue("filename")
    
    if filename:
        file_path = os.path.join(upload_dir, filename)
        
        if os.path.exists(file_path) and os.path.dirname(os.path.abspath(file_path)).endswith("upload"):
            os.remove(file_path)
            print("""
            <html>
            <head>
                <meta charset="UTF-8">
                <meta http-equiv="refresh" content="2;url=list_files.py">
            </head>
            <body>
                <h2>Archivo eliminado correctamente</h2>
                <p>Redirigiendo...</p>
            </body>
            </html>
            """)
        else:
            print("<h2>Error: El archivo no existe o no está permitido eliminarlo</h2>")
    else:
        print("<h2>Error: No se especificó ningún archivo</h2>")

except Exception as e:
    print(f"<h2>Error: {str(e)}</h2>") 