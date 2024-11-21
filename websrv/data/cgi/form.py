#!/usr/bin/env python3

import os
import cgi
import sys

# Configure encoding
sys.stdout.reconfigure(encoding='utf-8')

# Get request method
request_method = os.environ.get('REQUEST_METHOD', 'GET')

# Print HTTP headers
print("Content-Type: text/html\r\n\r\n", end='')

# Process form if POST
form = cgi.FieldStorage()

print("""<!DOCTYPE html>
<html>
<head>
    <title>CGI Response</title>
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
        }
        h2 {
            color: #333;
            border-bottom: 2px solid #4CAF50;
            padding-bottom: 10px;
        }
        .data-block {
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            margin-bottom: 20px;
        }
        .success { color: #4CAF50; }
        .info { color: #333; }
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
""")

if request_method == 'POST':
    print('<h1 class="success">Data Received!</h1>')
    print('<div class="data-block">')
    print('<h2>Form Data:</h2>')
    
    name = form.getvalue('name', 'Not provided')
    message = form.getvalue('message', 'Not provided')
    
    print(f'<p><strong>Name:</strong> {name}</p>')
    print(f'<p><strong>Message:</strong> {message}</p>')
    print('</div>')
else:
    print('<h1 class="info">GET Test Successful</h1>')

print("""
    <div class="container">
        <h2>Server Information</h2>
        <div class="data-block">
            <ul>
                <li><strong>Method:</strong> {}</li>
                <li><strong>Python Version:</strong> {}</li>
                <li><strong>Server Time:</strong> {}</li>
                <li><strong>Script Path:</strong> {}</li>
            </ul>
        </div>
        <p><a href="/cgi/index.html" style="color: #4CAF50; text-decoration: none;">&larr; Back to form</a></p>
    </div>
</body>
</html>""".format(
    os.environ.get('REQUEST_METHOD', 'Not available'),
    sys.version.split()[0],
    os.popen('date').read().strip(),
    os.environ.get('SCRIPT_NAME', 'Not available'),
)) 