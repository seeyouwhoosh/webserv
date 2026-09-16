#!/usr/bin/env python3
import sys
import os
import urllib.parse

# 1. Cabecera HTTP obligatoria que debe devolver el CGI
print("Content-Type: text/html; charset=utf-8\r\n\r\n")

# 2. Leer datos si es un método POST
post_data = ""
if os.environ.get("REQUEST_METHOD") == "POST":
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    post_data = sys.stdin.read(content_length)

# 3. Leer parámetros GET (QUERY_STRING)
query_string = os.environ.get("QUERY_STRING", "")
get_params = urllib.parse.parse_qs(query_string)
post_params = urllib.parse.parse_qs(post_data)

# 4. Generar la respuesta HTML
print("""
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <title>CGI Python - Webserv</title>
    <style>
        body { font-family: Arial, sans-serif; background: #0f172a; color: #f8fafc; padding: 20px; }
        .box { background: #1e293b; padding: 20px; border-radius: 8px; border: 1px solid #334155; }
        h1 { color: #38bdf8; }
        pre { background: #0f172a; padding: 10px; border-radius: 4px; color: #22c55e; }
    </style>
</head>
<body>
    <div class="box">
""")

print("<h1>¡Ejecución CGI Python Exitosa!</h1>")
print(f"<p><strong>Método HTTP:</strong> {os.environ.get('REQUEST_METHOD')}</p>")
print(f"<p><strong>Script ejecutado:</strong> {os.environ.get('SCRIPT_FILENAME')}</p>")

print("<h3>Parámetros GET recibidos:</h3>")
print(f"<pre>{get_params if get_params else 'Ninguno'}</pre>")

print("<h3>Datos POST recibidos:</h3>")
print(f"<pre>{post_data if post_data else 'Ninguno'}</pre>")

print("<h3>Variables de Entorno Clave:</h3>")
print("<ul>")
for key in ["SERVER_SOFTWARE", "SERVER_PROTOCOL", "REQUEST_URI", "QUERY_STRING", "CONTENT_LENGTH"]:
    print(f"<li><strong>{key}:</strong> {os.environ.get(key, 'N/A')}</li>")
print("</ul>")

print("""
    </div>
</body>
</html>
""")