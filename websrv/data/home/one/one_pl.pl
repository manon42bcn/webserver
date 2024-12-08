#!/usr/bin/perl

use strict;
use warnings;
use Time::HiRes qw(usleep);

# Imprimir cabeceras HTTP
print "Content-Type: text/html\r\n\r\n";

# Inicio del contenido HTML
print "<html><body>\n";
print "<h1>Perl CGI Script - From one.pl</h1>\n";
print "<h1>I'm forced to say:</h1>\n";

while (1) {
    print "<p>Waiting until timeout</p>\n";
    usleep(2_000_000);
}

my $path_says = $ENV{'PATH_INFO'} || "";

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
print "</body></html>\n";
