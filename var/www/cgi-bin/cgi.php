<?php
// 1. CGI headers
header("Content-Type: text/html; charset=utf-8");

// 2. Environment variables
$method = $_SERVER['REQUEST_METHOD'] ?? 'GET';
$query_string = $_SERVER['QUERY_STRING'] ?? '';
$content_length = $_SERVER['CONTENT_LENGTH'] ?? '0';
$content_type = $_SERVER['CONTENT_TYPE'] ?? 'Not specified';

$server_software = $_SERVER['SERVER_SOFTWARE'] ?? 'N/A';
$gateway_interface = $_SERVER['GATEWAY_INTERFACE'] ?? 'N/A';
$server_protocol = $_SERVER['SERVER_PROTOCOL'] ?? 'N/A';
$server_port = $_SERVER['SERVER_PORT'] ?? 'N/A';
$server_name = $_SERVER['SERVER_NAME'] ?? 'N/A';

$path_info = $_SERVER['PATH_INFO'] ?? 'N/A';
$path_translated = $_SERVER['PATH_TRANSLATED'] ?? 'N/A';
$script_name = $_SERVER['SCRIPT_NAME'] ?? 'N/A';
$remote_addr = $_SERVER['REMOTE_ADDR'] ?? 'N/A';

// Function to escape HTML values
function h($value) {
    if (!is_string($value)) {
        $value = (string)$value;
    }

    return htmlspecialchars(
        $value,
        ENT_QUOTES | ENT_SUBSTITUTE,
        'UTF-8'
    );
}

// 3. Process GET parameters
$get_data = [];

if ($query_string !== '') {
    parse_str($query_string, $get_data);
}

// 4. Read and parse the body from STDIN
$post_data = $_POST;
$raw_body = file_get_contents('php://input');

if ($raw_body === false) {
    $raw_body = '';
}
$body_bytes_received = 0;
$json_error = null;

if ($method === 'POST' || $method === 'PUT' || $method === 'PATCH') {
    $length = (int)$content_length;

    if ($length > 0) {
        // Read exactly CONTENT_LENGTH bytes.
        while ($body_bytes_received < $length) {
            $chunk = fread(STDIN, min(8192, $length - $body_bytes_received));

            if ($chunk === false || $chunk === '') {
                break;
            }

            $raw_body .= $chunk;
            $body_bytes_received += strlen($chunk);
        }

        if ($body_bytes_received < $length) {
            $post_data = [
                'error' => 'Incomplete request body',
                'expected_bytes' => $length,
                'received_bytes' => $body_bytes_received,
                'raw_body' => $raw_body
            ];
        } elseif (stripos($content_type, 'application/json') !== false) {
            // JSON sent directly in the request body.
            $decoded = json_decode($raw_body, true);

            if (json_last_error() === JSON_ERROR_NONE) {
                $post_data = $decoded;
            } else {
                $json_error = function_exists('json_last_sendError')
                    ? json_last_sendError()
                    : 'Invalid JSON';

                $post_data = [
                    'raw_body' => $raw_body,
                    'json_error' => $json_error
                ];
            }
        } elseif (
            stripos($content_type, 'application/x-www-form-urlencoded') !== false
        ) {
            // URL-encoded form data.
            parse_str($raw_body, $parsed_post);
            $post_data = $parsed_post;

            // If the form contains a raw_body field with JSON,
            // attempt to decode that field.
            if (isset($parsed_post['raw_body'])) {
                $decoded = json_decode($parsed_post['raw_body'], true);

                if (json_last_error() === JSON_ERROR_NONE) {
                    $post_data['raw_body_decoded'] = $decoded;
                }
            }
        } elseif (stripos($content_type, 'multipart/form-data') !== false) {
            // Multipart requires parsing MIME parts.
            // PHP CGI may expose the fields through $_POST.
            $post_data = [
                'raw_body_bytes' => $body_bytes_received,
                'multipart_fields' => $_POST,
                'multipart_files' => array_keys($_FILES),
                'note' => 'Check $_POST and $_FILES for fields and files'
            ];
        } else {
            // Unknown content type: preserve the body without transforming it.
            $post_data = [
                'raw_body' => $raw_body,
                'content_type' => $content_type
            ];
        }
    } else {
        $post_data = 'Empty body (CONTENT_LENGTH is 0)';
    }
}

// 5. Additional HTTP headers
$http_headers = [];

foreach ($_SERVER as $key => $value) {
    if (strpos($key, 'HTTP_') === 0) {
        $http_headers[$key] = $value;
    }
}

// Safe JSON representation for displaying in HTML.
function json_for_html($value) {
    $json = json_encode(
        $value,
        JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES
    );

    return $json !== false ? h($json) : 'Unable to generate JSON';
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>CGI PHP - Webserv Environment Test</title>
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

        h1 {
            color: #38bdf8;
        }

        h3 {
            color: #38bdf8;
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

        .ok {
            color: #22c55e;
        }

        .error {
            color: #f87171;
        }

        textarea {
            box-sizing: border-box;
            width: 100%;
            padding: 10px;
            border-radius: 4px;
            background: #0f172a;
            color: #f8fafc;
            border: 1px solid #334155;
        }

        button {
            margin-top: 10px;
            padding: 10px 16px;
            border: 0;
            border-radius: 4px;
            background: #0284c7;
            color: white;
            cursor: pointer;
        }
    </style>
</head>
<body>
<div class="box">
    <h1>CGI Environment Test (PHP)</h1>

    <p><strong>HTTP Method:</strong> <?= h($method) ?></p>
    <p><strong>Executed Script (SCRIPT_NAME):</strong> <?= h($script_name) ?></p>

    <h3>Received GET Parameters:</h3>
    <pre><?= json_for_html($get_data ?: 'None') ?></pre>

    <h3>Request Body Data (POST / STDIN):</h3>

    <pre><?= is_array($post_data)
        ? json_for_html($post_data)
        : h($post_data) ?></pre>

    <h3>Main Environment Variables (fill_env):</h3>
    <ul>
        <li><strong>SERVER_SOFTWARE:</strong> <?= h($server_software) ?></li>
        <li><strong>GATEWAY_INTERFACE:</strong> <?= h($gateway_interface) ?></li>
        <li><strong>SERVER_PROTOCOL:</strong> <?= h($server_protocol) ?></li>
        <li><strong>SERVER_PORT:</strong> <?= h($server_port) ?></li>
        <li><strong>SERVER_NAME:</strong> <?= h($server_name) ?></li>
        <li><strong>REQUEST_METHOD:</strong> <?= h($method) ?></li>
        <li><strong>QUERY_STRING:</strong> <?= h($query_string !== '' ? $query_string : 'N/A') ?></li>
        <li><strong>CONTENT_LENGTH:</strong> <?= h($content_length) ?></li>
        <li><strong>CONTENT_TYPE:</strong> <?= h($content_type) ?></li>
        <li><strong>PATH_INFO:</strong> <?= h($path_info) ?></li>
        <li><strong>PATH_TRANSLATED:</strong> <?= h($path_translated) ?></li>
        <li><strong>REMOTE_ADDR / HOST:</strong> <?= h($remote_addr) ?></li>
    </ul>

    <h3>Additional HTTP Headers (HTTP_*):</h3>
    <pre><?= json_for_html($http_headers ?: 'No additional headers') ?></pre>

    <h3>Test a JSON POST Request:</h3>
    <form method="POST" enctype="application/x-www-form-urlencoded">
        <textarea name="raw_body" rows="4">{"action":"execute","payload":"sample_data"}</textarea>
        <button type="submit">Submit Form</button>
    </form>
</div>
</body>
</html>
