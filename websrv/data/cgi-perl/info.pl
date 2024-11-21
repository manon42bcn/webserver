#!/usr/bin/perl

print "Content-type: text/html\n\n";

print <<EOF;
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <title>Respuesta CGI Perl</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 40px;
            background-color: #f0f0f0;
        }
        .container {
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
            max-width: 600px;
            margin: 0 auto;
        }
        .info-item {
            margin: 10px 0;
            padding: 10px;
            background-color: #f8f9fa;
            border-radius: 4px;
        }
        a {
            display: inline-block;
            margin-top: 20px;
            color: #4CAF50;
            text-decoration: none;
        }
        a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Server Information</h1>
EOF

print "<div class='info-item'><strong>Date and Time:</strong> " . localtime() . "</div>\n";
print "<div class='info-item'><strong>HTTP Method:</strong> $ENV{REQUEST_METHOD}</div>\n";
print "<div class='info-item'><strong>Gateway Interface:</strong> $ENV{GATEWAY_INTERFACE}</div>\n";
print "<div class='info-item'><strong>Server Protocol:</strong> $ENV{SERVER_PROTOCOL}</div>\n";
print "<div class='info-item'><strong>Script Name:</strong> $ENV{SCRIPT_NAME}</div>\n";
print "<div class='info-item'><strong>Server Name:</strong> $ENV{SERVER_NAME}</div>\n";
print "<div class='info-item'><strong>Server Port:</strong> $ENV{SERVER_PORT}</div>\n";

print <<EOF;
        <a href="index.html">← Back</a>
    </div>
</body>
</html>
EOF