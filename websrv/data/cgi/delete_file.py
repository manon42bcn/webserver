#!/usr/bin/env python3
import os
import cgi
import cgitb

cgitb.enable()

print("Content-Type: text/html")
print()

try:
    script_dir = os.path.dirname(os.path.abspath(__file__))
    upload_dir = os.path.join(script_dir, "upload")

    form = cgi.FieldStorage()
    filename = form.getvalue("filename")
    
    if filename:
        file_path = os.path.join(upload_dir, filename)
        
        if os.path.exists(file_path) and os.path.dirname(os.path.abspath(file_path)).endswith("upload"):
            os.remove(file_path)
            print("""
            <!DOCTYPE html>
            <html lang="en">
            <head>
                <meta charset="UTF-8">
                <meta http-equiv="refresh" content="2;url=/cgi/file_management.html">
                <title>File Deleted</title>
                <style>
                    body {
                        font-family: Arial, sans-serif;
                        margin: 40px;
                        max-width: 800px;
                        margin: 0 auto;
                        padding: 20px;
                    }
                    .container {
                        background-color: #f9f9f9;
                        padding: 20px;
                        border-radius: 8px;
                        box-shadow: 0 2px 4px rgba(0,0,0,0.1);
                        margin-bottom: 20px;
                        text-align: center;
                    }
                    h2 {
                        color: #333;
                        border-bottom: 2px solid #4CAF50;
                        padding-bottom: 10px;
                    }
                    p {
                        color: #666;
                        font-style: italic;
                        padding: 10px;
                    }
                    .logo {
                        width: 150px;
                        height: auto;
                        margin: 10px auto;
                    }
                </style>
            </head>
            <body>
                <div class="logo">
                    <img src="/cgi/logo.png" alt="Logo" class="logo">
                </div>
                <div class="container">
                    <h2>File successfully deleted</h2>
                    <p>Redirecting...</p>
                </div>
            </body>
            </html>
            """)
        else:
            print("""
            <!DOCTYPE html>
            <html lang="en">
            <head>
                <meta charset="UTF-8">
                <title>Error</title>
                <style>
                    body {
                        font-family: Arial, sans-serif;
                        margin: 40px;
                        max-width: 800px;
                        margin: 0 auto;
                        padding: 20px;
                    }
                    .container {
                        background-color: #f9f9f9;
                        padding: 20px;
                        border-radius: 8px;
                        box-shadow: 0 2px 4px rgba(0,0,0,0.1);
                        margin-bottom: 20px;
                        text-align: center;
                    }
                    h2 {
                        color: #ff4444;
                        border-bottom: 2px solid #ff4444;
                        padding-bottom: 10px;
                    }
                    .logo {
                        width: 150px;
                        height: auto;
                        margin: 10px auto;
                    }
                </style>
            </head>
            <body>
                <div class="logo">
                    <img src="/cgi/logo.png" alt="Logo" class="logo">
                </div>
                <div class="container">
                    <h2>Error: File does not exist or deletion is not allowed</h2>
                </div>
            </body>
            </html>
            """)
    else:
        print("<h2>Error: No file specified</h2>")

except Exception as e:
    print(f"<h2>Error: {str(e)}</h2>") 