#!/usr/bin/perl -wT
use strict;
use warnings;

# Clear the environment for security because of -T (Taint Mode)
delete @ENV{qw(IFS CDPATH ENV BASH_ENV)};
$ENV{PATH} = '/usr/local/bin:/bin:/usr/bin';

# --------------------------------------------------
# Helper functions
# --------------------------------------------------

sub url_decode {
    my ($value) = @_;
    $value = '' unless defined $value;

    $value =~ tr/+/ /;
    $value =~ s/%([0-9a-fA-F]{2})/chr(hex($1))/eg;

    return $value;
}

sub escape_html {
    my ($value) = @_;
    $value = '' unless defined $value;

    $value =~ s/&/&amp;/g;
    $value =~ s/</&lt;/g;
    $value =~ s/>/&gt;/g;
    $value =~ s/"/&quot;/g;
    $value =~ s/'/&#39;/g;

    return $value;
}

# Basic JSON encoding without external modules.
sub json_quote {
    my ($value) = @_;
    $value = '' unless defined $value;

    $value =~ s/\\/\\\\/g;
    $value =~ s/"/\\"/g;
    $value =~ s/\x08/\\b/g;
    $value =~ s/\n/\\n/g;
    $value =~ s/\r/\\r/g;
    $value =~ s/\t/\\t/g;
    $value =~ s/\f/\\f/g;

    return '"' . $value . '"';
}

sub json_value {
    my ($value) = @_;

    return 'null' unless defined $value;

    if (ref($value) eq 'HASH') {
        my @pairs;

        foreach my $key (sort keys %{$value}) {
            push @pairs,
                json_quote($key) . ': ' . json_value($value->{$key});
        }

        return '{' . join(', ', @pairs) . '}';
    }

    if (ref($value) eq 'ARRAY') {
        return '[' . join(', ', map { json_value($_) } @{$value}) . ']';
    }

    return json_quote($value);
}

sub parse_urlencoded {
    my ($input) = @_;
    my %result;

    foreach my $pair (split /&/, $input) {
        next if $pair eq '';

        my ($key, $value) = split /=/, $pair, 2;

        $key   = url_decode($key);
        $value = url_decode($value);

        # Keep the last value if a key appears multiple times.
        $result{$key} = $value;
    }

    return %result;
}

# --------------------------------------------------
# 1. CGI environment variables
# --------------------------------------------------

my $method            = $ENV{REQUEST_METHOD} || 'GET';
my $query_string      = $ENV{QUERY_STRING} || '';
my $content_length    = $ENV{CONTENT_LENGTH} || '0';
my $content_type      = $ENV{CONTENT_TYPE} || 'Not specified';

my $server_software   = $ENV{SERVER_SOFTWARE} || 'N/A';
my $gateway_interface = $ENV{GATEWAY_INTERFACE} || 'N/A';
my $server_protocol   = $ENV{SERVER_PROTOCOL} || 'N/A';
my $server_port       = $ENV{SERVER_PORT} || 'N/A';
my $server_name       = $ENV{SERVER_NAME} || 'N/A';

my $path_info         = $ENV{PATH_INFO} || 'N/A';
my $path_translated   = $ENV{PATH_TRANSLATED} || 'N/A';
my $script_name       = $ENV{SCRIPT_NAME} || 'N/A';
my $remote_addr       = $ENV{REMOTE_ADDR} || 'N/A';

# --------------------------------------------------
# 2. Read the request body from STDIN
# --------------------------------------------------

my $post_body  = '';
my $post_error = '';

if ($method eq 'POST') {
    if ($content_length !~ /^\d+$/) {
        $post_error = 'Invalid CONTENT_LENGTH';
    }
    elsif ($content_length > 0) {
        binmode(STDIN);

        while (length($post_body) < $content_length) {
            my $chunk = '';
            my $remaining = $content_length - length($post_body);
            my $bytes_read = read(STDIN, $chunk, $remaining);

            if (!defined $bytes_read) {
                $post_error = "Error reading STDIN: $!";
                last;
            }

            if ($bytes_read == 0) {
                $post_error = 'Unexpected EOF: received '
                    . length($post_body) . " of $content_length bytes";
                last;
            }

            $post_body .= $chunk;
        }
    }
}

# --------------------------------------------------
# 3. Process GET parameters
# --------------------------------------------------

my %get_data = parse_urlencoded($query_string);

# --------------------------------------------------
# 4. Process POST data
# --------------------------------------------------

my $post_data = 'None';

if ($method eq 'POST') {
    if ($post_error ne '') {
        $post_data = { error => $post_error };
    }
    elsif ($content_length == 0) {
        $post_data = 'Empty body (CONTENT_LENGTH is 0)';
    }
    elsif ($content_type =~ m{application/json}i) {
        # JSON is not parsed; the received content is displayed as-is.
        $post_data = $post_body;
    }
    elsif ($content_type =~ m{application/x-www-form-urlencoded}i) {
        my %form_data = parse_urlencoded($post_body);

        if (exists $form_data{raw_body}) {
            # Decode raw_body without parsing its JSON content.
            $post_data = $form_data{raw_body};
        }
        else {
            $post_data = \%form_data;
        }
    }
    else {
        $post_data = $post_body;
    }
}

# --------------------------------------------------
# 5. Additional HTTP headers
# --------------------------------------------------

my %http_headers;

foreach my $env_key (keys %ENV) {
    if ($env_key =~ /^HTTP_(.*)$/) {
        $http_headers{$env_key} = $ENV{$env_key};
    }
}

# --------------------------------------------------
# 6. Convert data to JSON without JSON.pm
# --------------------------------------------------

my $get_json = %get_data
    ? json_value(\%get_data)
    : json_quote('None');

my $post_json = json_value($post_data);
my $head_json = json_value(\%http_headers);

# --------------------------------------------------
# 7. Escape values for HTML
# --------------------------------------------------

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

# --------------------------------------------------
# 8. Generate the CGI response
# --------------------------------------------------

print "Content-Type: text/html; charset=utf-8\r\n\r\n";

print <<HTML;
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>CGI Perl - Webserv Environment Test</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #0f172a;
            color: #f8fafc;
            padding: 20px;
        }
        .box {
            background: #1e293b;
            padding: 20px;
            border-radius: 8px;
            border: 1px solid #334155;
            max-width: 900px;
            margin: auto;
        }
        h1, h3 {
            color: #38bdf8;
        }
        h3 {
            border-bottom: 1px solid #334155;
            padding-bottom: 5px;
            margin-top: 25px;
        }
        pre {
            background: #0f172a;
            padding: 10px;
            border-radius: 4px;
            color: #22c55e;
            overflow-x: auto;
            white-space: pre-wrap;
            overflow-wrap: anywhere;
        }
        ul {
            line-height: 1.6;
        }
    </style>
</head>
<body>
    <div class="box">
        <h1>CGI Environment Test (Perl -T)</h1>

        <p><strong>HTTP Method:</strong> $method</p>
        <p><strong>Executed Script (SCRIPT_NAME):</strong> $script_name</p>

        <h3>Received GET Parameters:</h3>
        <pre>$get_json</pre>

        <h3>Request Body Data (POST / STDIN):</h3>
        <pre>$post_json</pre>

        <h3>Main Environment Variables (fill_env):</h3>
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

        <h3>Additional HTTP Headers (HTTP_*):</h3>
        <pre>$head_json</pre>
    </div>
</body>
</html>
HTML