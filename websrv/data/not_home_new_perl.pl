#!/usr/bin/perl

use strict;
use warnings;

# Obtener PATH_INFO del entorno
my $path_says = $ENV{'PATH_INFO'} || "";

# Imprimir cabeceras HTTP
# print "Content-Type: text/html\r\n\r\n";

# Imprimir contenido HTML
print "<html><body>\n";
print "<h1>Perl CGI Script - From NO_HOME.pl</h1>\n";
print "<h1>I'm forced to say:</h1>\n";

# Procesar PATH_INFO
if (length($path_says) > 1) {
    my @path = split('/', $path_says);
    foreach my $elem (@path) {
        print "<h3>$elem</h3>\n";
    }
}

print "<h1>Sorry... not my fault, that was path's fault..</h1>\n";

# Imprimir información del entorno
print "<p>Request Method: " . ($ENV{'REQUEST_METHOD'} || "unknown") . "</p>\n";
print "<p>Query String: " . ($ENV{'QUERY_STRING'} || "") . "</p>\n";
print "<p>Content Type: " . ($ENV{'CONTENT_TYPE'} || "") . "</p>\n";
print "<p>Content Length: " . ($ENV{'CONTENT_LENGTH'} || "") . "</p>\n";
print "<p>Path info: " . ($ENV{'PATH_INFO'} || "") . "</p>\n";
print "<p>Script Name: " . ($ENV{'SCRIPT_NAME'} || "") . "</p>\n";
print "<p>Server Name: " . ($ENV{'SERVER_NAME'} || "") . "</p>\n";
print "<p>Server Port: " . ($ENV{'SERVER_PORT'} || "") . "</p>\n";
print "</body></html>\n";
