#!/usr/bin/php-cgi
<?php
// PHP-CGI imprime automáticamente las cabeceras Content-Type si se estructuran así:
echo "Content-Type: text/html; charset=utf-8\n\n";
?>
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <title>CGI PHP - Webserv</title>
    <style>
        body { font-family: Arial, sans-serif; background: #0f172a; color: #f8fafc; padding: 20px; }
        .box { background: #1e293b; padding: 20px; border-radius: 8px; border: 1px solid #334155; }
        h1 { color: #f43f5e; }
        pre { background: #0f172a; padding: 10px; border-radius: 4px; color: #38bdf8; }
    </style>
</head>
<body>
    <div class="box">
        <h1>¡Ejecución CGI PHP Exitosa!</h1>
        <p><strong>Método HTTP:</strong> <?php echo $_SERVER['REQUEST_METHOD']; ?></p>
        
        <h3>Parámetros GET ($_GET):</h3>
        <pre><?php print_r($_GET); ?></pre>

        <h3>Parámetros POST ($_POST):</h3>
        <pre><?php print_r($_POST); ?></pre>

        <h3>Entorno del Servidor:</h3>
        <p><strong>Query String:</strong> <?php echo $_SERVER['QUERY_STRING'] ?? 'Ninguna'; ?></p>
        <p><strong>User Agent:</strong> <?php echo $_SERVER['HTTP_USER_AGENT'] ?? 'Desconocido'; ?></p>
    </div>
</body>
</html>