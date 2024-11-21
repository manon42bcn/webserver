#!/usr/bin/env python3

import os
import sys
import cgi
import cgitb
import io

cgitb.enable(display=0, logdir="/tmp")

DEBUG = False 
UPLOAD_DIR = os.path.join(os.path.dirname(__file__), 'upload')

def save_file(post_data):
    try:
        first_line = post_data.split(b'\r\n')[0]
        boundary = first_line.decode()

        temp_file = io.BytesIO(post_data)

        environ = os.environ.copy()
        environ['CONTENT_TYPE'] = f'multipart/form-data; boundary={boundary[2:]}'

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

        filename = os.path.basename(fileitem.filename)
        filepath = os.path.join(UPLOAD_DIR, filename)

        with open(filepath, 'wb') as f:
            if hasattr(fileitem.file, 'read'):
                while True:
                    chunk = fileitem.file.read(8192)
                    if not chunk:
                        break
                    f.write(chunk)
            else:
                f.write(fileitem.value)

        return True, (filename)
    except Exception as e:
        return False, f"Error processing file: {str(e)}"

try:
    if not os.path.exists(UPLOAD_DIR):
        os.makedirs(UPLOAD_DIR, mode=0o755)

    if os.environ.get('REQUEST_METHOD') != 'POST':
        raise Exception("Method not allowed. Use POST to upload files.")

    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    post_data = sys.stdin.buffer.read(content_length)

    # debug_file = os.path.join(os.path.dirname(__file__), 'raw_post_data.txt')
    # with open(debug_file, 'wb') as f:
    #     f.write(post_data)

    sys.stdout.write("Content-Type: text/html\n\n")

    success, result = save_file(post_data)

    print(f"""<!DOCTYPE html>
    <html>
    <head>
        <title>Upload Result</title>
        <meta http-equiv="refresh" content="2;url=/cgi/file_management.html">
        <style>
            body {{
                font-family: Arial, sans-serif;
                margin: 40px;
                max-width: 800px;
                margin: 0 auto;
                padding: 20px;
            }}
            .container {{
                background-color: #f9f9f9;
                padding: 20px;
                border-radius: 8px;
                box-shadow: 0 2px 4px rgba(0,0,0,0.1);
                margin-bottom: 20px;
            }}
            h2 {{
                color: #333;
                border-bottom: 2px solid #4CAF50;
                padding-bottom: 10px;
            }}
            pre {{
                background: white;
                padding: 10px;
                border-radius: 4px;
                overflow-x: auto;
                border: 1px solid #ddd;
            }}
            .success {{
                background-color: white;
                padding: 20px;
                border-radius: 8px;
                margin-bottom: 20px;
                border-left: 4px solid #4CAF50;
            }}
            .error {{
                background-color: white;
                padding: 20px;
                border-radius: 8px;
                margin-bottom: 20px;
                border-left: 4px solid #ff4444;
            }}
            .back-link {{
                color: #4CAF50;
                text-decoration: none;
                display: inline-block;
                margin-top: 1rem;
            }}
            .back-link:hover {{
                color: #45a049;
            }}
            .message {{
                text-align: center;
                margin-top: 1rem;
                color: #666;
                font-style: italic;
            }}
            .logo {{
                width: 150px;
                height: auto;
                margin: 10px auto;
                display: block;
            }}
        </style>
    </head>
    <body>
        <div class="logo">
            <img src="/cgi/logo.png" alt="Logo" class="logo">
        </div>
        <div class="container">""")

    if DEBUG:
        print(f"""
        <h2>Debug Information:</h2>
        <pre>
Content-Length: {content_length}
Bytes read: {len(post_data)}

First 1024 bytes:
{post_data[:1024]}
        </pre>
        """)



    print("<h2>Upload Result</h2>")

    if success:
        filename = result
        print(f"""
        <div class="success">
            <h3>File Uploaded Successfully!</h3>
            <p><strong>File:</strong> {filename}</p>
        </div>
        <p class="message">Redirecting to home page in 2 seconds...</p>
        """)
    else:
        print(f"""
        <div class="error">
            <h3>Upload Failed</h3>
            <p>{result}</p>
        </div>
        <p class="message">Redirecting to home page in 2 seconds...</p>
        """)

    print("""
        </div>
    </body>
    </html>""")

except Exception as e:
    sys.stdout.write("Content-Type: text/html\n\n")
    print(f"""<!DOCTYPE html>
    <html>
    <head>
        <title>Error</title>
        <meta http-equiv="refresh" content="2;url=/cgi/file_management.html">
        <style>
            body {{
                font-family: Arial, sans-serif;
                margin: 40px;
                max-width: 800px;
                margin: 0 auto;
                padding: 20px;
            }}
            .container {{
                background-color: #f9f9f9;
                padding: 20px;
                border-radius: 8px;
                box-shadow: 0 2px 4px rgba(0,0,0,0.1);
                margin-bottom: 20px;
            }}
            .error {{
                background-color: white;
                padding: 20px;
                border-radius: 8px;
                margin-bottom: 20px;
                border-left: 4px solid #ff4444;
            }}
            h1 {{
                color: #333;
                border-bottom: 2px solid #4CAF50;
                padding-bottom: 10px;
            }}
            .message {{
                text-align: center;
                margin-top: 1rem;
                color: #666;
                font-style: italic;
            }}
            .logo {{
                width: 150px;
                height: auto;
                margin: 10px auto;
                display: block;
            }}
        </style>
    </head>
    <body>
        <div class="logo">
            <img src="/cgi/logo.png" alt="Logo" class="logo">
        </div>
        <div class="container">
            <h1>Server Error</h1>
            <div class="error">
                <p>{str(e)}</p>
            </div>
            <p class="message">Redirecting to home page in 2 seconds...</p>
        </div>
    </body>
    </html>
    """)