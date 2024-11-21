#!/usr/bin/env python3
import os
import cgi
import cgitb

cgitb.enable()

print("Content-Type: text/html")
print()

script_dir = os.path.dirname(os.path.abspath(__file__))
upload_dir = os.path.join(script_dir, "upload")

print("""<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <title>Administrar Archivos</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 40px;
        }
        .file-list {
            margin: 20px;
        }
        .file-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px;
            border-bottom: 1px solid #ddd;
        }
        .delete-btn {
            background-color: #ff4444;
            color: white;
            border: none;
            padding: 5px 10px;
            cursor: pointer;
            border-radius: 3px;
        }
        .delete-btn:hover {
            background-color: #cc0000;
        }
        .no-files {
            color: #666;
            font-style: italic;
            margin: 20px;
        }
        .back-link {
            display: inline-block;
            margin: 20px;
            color: #0066cc;
            text-decoration: none;
        }
        .back-link:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <h1>Archivos en la carpeta Upload</h1>
    <div class="file-list">""")

try:
    print(f'<p>Searching files in: {upload_dir}</p>')  # Debug info
    if os.path.exists(upload_dir):
        files = os.listdir(upload_dir)
        if files:
            for file in files:
                file_path = os.path.join(upload_dir, file)
                if os.path.isfile(file_path):
                    print(f"""
                        <div class="file-item">
                            <span>{file}</span>
                            <form method="post" action="/cgi/delete_file.py" style="display: inline;" onsubmit="return confirm('Are you sure you want to delete this file?');">
                                <input type="hidden" name="filename" value="{file}">
                                <button type="submit" class="delete-btn">Delete</button>
                            </form>
                        </div>""")
        else:
            print('<p class="no-files">No files in folder</p>')
    else:
        print(f'<p class="no-files">The folder upload does not exist in the path: {upload_dir}</p>')
except Exception as e:
    print(f'<p class="error">Error listing files: {str(e)}</p>')

print("""
    </div>
    <a href="/cgi/" class="back-link">&larr; Back to main page of CGI</a>
</body>
</html>""") 