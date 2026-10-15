#!/usr/bin/perl -wT
use strict;
use warnings;
use CGI;
use JSON;

# Limpiar el entorno por seguridad debido a -T (Taint Mode)
delete @ENV{qw(IFS CDPATH ENV BASH_ENV)};
$ENV{PATH} = '/usr/local/bin:/bin:/usr/bin';

# 1. Recoger variables de entorno CGI
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

# 2. Leer el body ANTES de crear CGI->new
my $post_body = '';
my $post_error = '';

if ($method eq 'POST') {
    if ($content_length !~ /^\d+$/) {
        $post_error = 'CONTENT_LENGTH inválido';
    }
    elsif ($content_length > 0) {
        binmode(STDIN);

        while (length($post_body) < $content_length) {
            my $chunk = '';
            my $remaining = $content_length - length($post_body);
            my $bytes_read = read(STDIN, $chunk, $remaining);

            if (!defined $bytes_read) {
                $post_error = "Error leyendo STDIN: $!";
                last;
            }

            if ($bytes_read == 0) {
                $post_error = 'EOF inesperado: se recibieron '
                    . length($post_body) . " de $content_length bytes";
                last;
            }

            $post_body .= $chunk;
        }
    }
}

# 3. Crear CGI después de leer STDIN.
# CGI.pm puede procesar parámetros del formulario por su cuenta.
my $q = CGI->new;

# 4. Procesar parámetros GET
my %get_data;

if ($query_string ne '') {
    foreach my $pair (split /&/, $query_string) {
        my ($key, $value) = split /=/, $pair, 2;

        $key   = $q->unescape(defined $key   ? $key   : '');
        $value = $q->unescape(defined $value ? $value : '');

        $get_data{$key} = $value;
    }
}

# 5. Procesar el body POST
my $post_data = 'Ninguno';

if ($method eq 'POST') {
    if ($post_error ne '') {
        $post_data = "Error: $post_error";
    }
    elsif ($content_length == 0) {
        $post_data = 'Cuerpo vacío (CONTENT_LENGTH es 0)';
    }
    else {
        $post_data = $post_body;
    }
}

# 6. Filtrar cabeceras HTTP extra (HTTP_*)
my %http_headers;

foreach my $env_key (keys %ENV) {
    if ($env_key =~ /^HTTP_(.*)$/) {
        $http_headers{$env_key} = $ENV{$env_key};
    }
}

# 7. Generar JSON
my $json = JSON->new->allow_nonref->utf8->pretty;

my $get_json  = %get_data ? $json->encode(\%get_data) : '"Ninguno"';
my $head_json = $json->encode(\%http_headers);
use HTML::Entities;

my $post_json = encode_entities($post_data, '<>&');
# 8. Escapar valores interpolados en HTML
sub escape_html {
    my ($s) = @_;
    $s = '' unless defined $s;
    $s =~ s/&/&amp;/g;
    $s =~ s/</&lt;/g;
    $s =~ s/>/&gt;/g;
    $s =~ s/"/&quot;/g;
    $s =~ s/'/&#39;/g;
    return $s;
}

my @html_values = (
    $method, $script_name, $server_software, $gateway_interface,
    $server_protocol, $server_port, $server_name,
    $query_string || 'N/A', $content_length, $content_type,
    $path_info, $path_translated, $remote_addr
);

@html_values = map { escape_html($_) } @html_values;

(
    $method, $script_name, $server_software, $gateway_interface,
    $server_protocol, $server_port, $server_name,
    $query_string, $content_length, $content_type,
    $path_info, $path_translated, $remote_addr
) = @html_values;

$get_json  = escape_html($get_json);
$post_json = escape_html($post_json);
$head_json = escape_html($head_json);

# 9. Generar HTML
print $q->header(-type => 'text/html', -charset => 'utf-8');

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
        <h1>¡Prueba de Entorno CGI (Perl -T)!</h1>
        <p><strong>Método HTTP:</strong> $method</p>
        <p><strong>Script ejecutado (SCRIPT_NAME):</strong> $script_name</p>

        <h3>Parámetros GET recibidos:</h3>
        <pre>$get_json</pre>

        <h3>Datos del Cuerpo (POST / STDIN):</h3>
        <pre>$post_json</pre>

        <h3>Variables de Entorno Principales (fill_env):h3>
        <ul>
            <li><strong>SERVER_SOFTWARE:</strong> $server_software</li>
            <li><strong>GATEWAY_INTERFACE:</strong> $gateway_interface</li>
            <li><strong>SERVER_PROTOCOL:</strong> $server_protocol</li>
            <li><strong>SERVER_PORT:</strong> $server_port</li>
            <li><strong>SERVER_NAME:</strong> $server_name</li>
            <li><strong>REQUEST_METHOD:</strong> $method</li>
            <li><strong>QUERY_STRING:</strong> $query_string</li>
            <li><strong>CONTENT_LENGTH:</strong> $content_length</li>
            <li><strong>CONTENT_TYPE:</strong> $content_type</li>
            <li><strong>PATH_INFO:</strong> $path_info</li>
            <li><strong>PATH_TRANSLATED:</strong> $path_translated</li>
            <li><strong>REMOTE_ADDR / HOST:</strong> $remote_addr</li>
        </ul>

        <h3>Cabeceras HTTP extra (HTTP_*):</h3>
        <pre>$head_json</pre>
    </div>
</body>
</html>
HTML