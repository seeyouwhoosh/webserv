#!/usr/bin/perl -wT
use strict;
use warnings;
use CGI;
use JSON;

# Limpiar el entorno por seguridad debido a -T (Taint Mode)
delete @ENV{qw(IFS CDPATH ENV BASH_ENV)};
$ENV{PATH} = '/usr/local/bin:/bin:/usr/bin';

my $q = CGI->new;

# 1. Cabeceras HTTP obligatorias
print $q->header(-type => 'text/html', -charset => 'utf-8');

# 2. Recoger variables de entorno inyectadas
my $method            = $ENV{REQUEST_METHOD} || 'GET';
my $query_string      = $ENV{QUERY_STRING} || '';
my $content_length    = $ENV{CONTENT_LENGTH} || 0;
my $content_type      = $ENV{CONTENT_TYPE} || 'No especificado';

my $server_software   = $ENV{SERVER_SOFTWARE} || 'N/A';
my $gateway_interface = $ENV{GATEWAY_INTERFACE} || 'N/A';
my $server_protocol   = $ENV{SERVER_PROTOCOL} || 'N/A';
my $server_port       = $ENV{SERVER_PORT} || 'N/A';
my $server_name       = $ENV{SERVER_NAME} || 'N/A';

my $path_info         = $ENV{PATH_INFO} || 'N/A';
my $path_translated   = $ENV{PATH_TRANSLATED} || 'N/A';
my $script_name       = $ENV{SCRIPT_NAME} || 'N/A';
my $remote_addr       = $ENV{REMOTE_ADDR} || 'N/A';

# 3. Procesar parámetros GET
my %get_data;
if ($query_string) {
    foreach my $pair (split('&', $query_string)) {
        my ($key, $value) = split('=', $pair, 2);
        $key = $q->escapeHTML($q->unescape($key // ''));
        $value = $q->escapeHTML($q->unescape($value // ''));
        $get_data{$key} = $value;
    }
}

# 4. Procesar el cuerpo (POST / STDIN)
my $post_data = "Ninguno";
if ($method eq 'POST' && $content_length > 0) {
    binmode(STDIN);
    my $post_body;
    read(STDIN, $post_body, $content_length);
    
    if ($content_type =~ /application\/json/) {
        eval {
            $post_data = decode_json($post_body);
        };
        if ($@) {
            $post_data = "Error decodificando JSON: $@ (Raw: $post_body)";
        }
    } else {
        my %parsed_post;
        foreach my $pair (split('&', $post_body)) {
            my ($key, $value) = split('=', $pair, 2);
            $key = $q->escapeHTML($q->unescape($key // ''));
            $value = $q->escapeHTML($q->unescape($value // ''));
            $parsed_post{$key} = $value;
        }
        $post_data = keys %parsed_post ? \%parsed_post : $post_body;
    }
} elsif ($method eq 'POST') {
    $post_data = "Cuerpo vacío (CONTENT_LENGTH es 0)";
}

# 5. Filtrar cabeceras HTTP extra (HTTP_*)
my %http_headers;
foreach my $env_key (keys %ENV) {
    if ($env_key =~ /^HTTP_(.*)$/) {
        $http_headers{$env_key} = $ENV{$env_key};
    }
}

my $json = JSON->new->allow_nonref->utf8->pretty;

# 6. Generar HTML
print <<HTML;
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <title>CGI Perl - Webserv Env Test</title>
    <style>
        body { font-family: Arial, sans-serif; background: #0f172a; color: #f8fafc; padding: 20px; }
        .box { background: #1e293b; padding: 20px; border-radius: 8px; border: 1px solid #334155; max-width: 900px; margin: auto; }
        h1 { color: #38bdf8; }
        h3 { color: #38bdf8; border-bottom: 1px solid #334155; padding-bottom: 5px; margin-top: 25px; }
        pre { background: #0f172a; padding: 10px; border-radius: 4px; color: #22c55e; overflow-x: auto; }
        ul { line-height: 1.6; }
    </style>
</head>
<body>
    <div class="box">
        <h1>¡Prueba de Entorno CGI (Perl -T) Exitosa!</h1>
        <p><strong>Método HTTP:</strong> $method</p>
        <p><strong>Script ejecutado (SCRIPT_NAME):</strong> $script_name</p>
        
        <h3>Parámetros GET recibidos:</h3>
        <pre>\%([% keys %get_data ? $json->encode(\%get_data) : '"Ninguno"' %])</pre>
        
        <h3>Datos del Cuerpo (POST / STDIN):</h3>
        <pre>[$json->encode($post_data)]</pre>
        
        <h3>Variables de Entorno Principales (fill_env):</h3>
        <ul>
            <li><strong>SERVER_SOFTWARE:</strong> $server_software</li>
            <li><strong>GATEWAY_INTERFACE:</strong> $gateway_interface</li>
            <li><strong>SERVER_PROTOCOL:</strong> $server_protocol</li>
            <li><strong>SERVER_PORT:</strong> $server_port</li>
            <li><strong>SERVER_NAME:</strong> $server_name</li>
            <li><strong>REQUEST_METHOD:</strong> $method</li>
            <li><strong>QUERY_STRING:</strong> @{[ $query_string ? $query_string : 'N/A' ]}</li>
            <li><strong>CONTENT_LENGTH:</strong> $content_length</li>
            <li><strong>CONTENT_TYPE:</strong> $content_type</li>
            <li><strong>PATH_INFO:</strong> $path_info</li>
            <li><strong>PATH_TRANSLATED:</strong> $path_translated</li>
            <li><strong>REMOTE_ADDR / HOST:</strong> $remote_addr</li>
        </ul>

        <h3>Cabeceras HTTP extra (HTTP_*):</h3>
        <pre>[$json->encode(\%http_headers)]</pre>
    </div>
</body>
</html>
HTML