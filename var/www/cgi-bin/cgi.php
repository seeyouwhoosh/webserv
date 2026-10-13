<?php
// 1. Cabeceras HTTP obligatorias
header("Content-Type: text/html; charset=utf-8");

// 2. Obtener variables de entorno
$method = $_SERVER['REQUEST_METHOD'] ?? 'GET';
$query_string = $_SERVER['QUERY_STRING'] ?? '';
$content_length = $_SERVER['CONTENT_LENGTH'] ?? '0';
$content_type = $_SERVER['CONTENT_TYPE'] ?? 'No especificado';

$server_software = $_SERVER['SERVER_SOFTWARE'] ?? 'N/A';
$gateway_interface = $_SERVER['GATEWAY_INTERFACE'] ?? 'N/A';
$server_protocol = $_SERVER['SERVER_PROTOCOL'] ?? 'N/A';
$server_port = $_SERVER['SERVER_PORT'] ?? 'N/A';
$server_name = $_SERVER['SERVER_NAME'] ?? 'N/A';

$path_info = $_SERVER['PATH_INFO'] ?? 'N/A';
$path_translated = $_SERVER['PATH_TRANSLATED'] ?? 'N/A';
$script_name = $_SERVER['SCRIPT_NAME'] ?? 'N/A';
$remote_addr = $_SERVER['REMOTE_ADDR'] ?? 'N/A';

// 3. Procesar GET
$get_data = [];
if (!empty($query_string)) {
    parse_str($query_string, $get_data);
}

// 4. Procesar POST / Cuerpo recibido por stdin
$post_data = "Ninguno";
if ($method === 'POST') {
    $length = intval($content_length);
    if ($length > 0) {
        $post_body = file_get_contents('php://stdin');
        if (strpos($content_type, 'application/json') !== false) {
            $post_data = json_decode($post_body, true) ?? $post_body;
        } else {
            parse_str($post_body, $parsed_post);
            $post_data = !empty($parsed_post) ? $parsed_post : $post_body;
        }
    } else {
        $post_data = "Cuerpo vacío (CONTENT_LENGTH es 0)";
    }
}

// 5. Filtrar cabeceras HTTP extra (HTTP_*)
$http_headers = [];
foreach ($_SERVER as $key => $value) {
    if (strpos($key, 'HTTP_') === 0) {
        $http_headers[$key] = $value;
    }
}
?>
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <title>CGI PHP - Webserv Env Test</title>
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
        <h1>¡Prueba de Entorno CGI (PHP) Exitosa!</h1>
        <p><strong>Método HTTP:</strong> <?php echo htmlspecialchars($method); ?></p>
        <p><strong>Script ejecutado (SCRIPT_NAME):</strong> <?php echo htmlspecialchars($script_name); ?></p>
        
        <h3>Parámetros GET recibidos:</h3>
        <pre><?php echo htmlspecialchars(json_encode($get_data, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE) ?: "Ninguno"); ?></pre>
        
        <h3>Datos del Cuerpo (POST / STDIN):</h3>
        <pre><?php 
            if (is_array($post_data)) {
                echo htmlspecialchars(json_encode($post_data, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE));
            } else {
                echo htmlspecialchars($post_data);
            }
        ?></pre>
        
        <h3>Variables de Entorno Principales (fill_env):</h3>
        <ul>
            <li><strong>SERVER_SOFTWARE:</strong> <?php echo htmlspecialchars($server_software); ?></li>
            <li><strong>GATEWAY_INTERFACE:</strong> <?php echo htmlspecialchars($gateway_interface); ?></li>
            <li><strong>SERVER_PROTOCOL:</strong> <?php echo htmlspecialchars($server_protocol); ?></li>
            <li><strong>SERVER_PORT:</strong> <?php echo htmlspecialchars($server_port); ?></li>
            <li><strong>SERVER_NAME:</strong> <?php echo htmlspecialchars($server_name); ?></li>
            <li><strong>REQUEST_METHOD:</strong> <?php echo htmlspecialchars($method); ?></li>
            <li><strong>QUERY_STRING:</strong> <?php echo htmlspecialchars($query_string ?: "N/A"); ?></li>
            <li><strong>CONTENT_LENGTH:</strong> <?php echo htmlspecialchars($content_length); ?></li>
            <li><strong>CONTENT_TYPE:</strong> <?php echo htmlspecialchars($content_type); ?></li>
            <li><strong>PATH_INFO:</strong> <?php echo htmlspecialchars($path_info); ?></li>
            <li><strong>PATH_TRANSLATED:</strong> <?php echo htmlspecialchars($path_translated); ?></li>
            <li><strong>REMOTE_ADDR / HOST:</strong> <?php echo htmlspecialchars($remote_addr); ?></li>
        </ul>

        <h3>Cabeceras HTTP extra (HTTP_*):</h3>
        <pre><?php echo htmlspecialchars(json_encode($http_headers, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE) ?: "Ninguna cabecera extra"); ?></pre>
    </div>
</body>
</html>