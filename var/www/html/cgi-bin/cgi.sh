#!/bin/bash

# Cabecera HTTP obligatoria
echo "Content-Type: text/html; charset=utf-8"
echo ""

echo "<!DOCTYPE html>"
echo "<html><head><title>CGI Bash</title></head>"
echo "<body style='background:#0f172a; color:white; font-family:sans-serif; padding:20px;'>"
echo "<div style='background:#1e293b; padding:20px; border-radius:8px;'>"
echo "<h1 style='color:#22c55e;'>¡CGI Bash ejecutado correctamente!</h1>"
echo "<p><strong>QUERY_STRING:</strong> ${QUERY_STRING:-Ninguna}</p>"
echo "<p><strong>REQUEST_METHOD:</strong> ${REQUEST_METHOD}</p>"
echo "<p><strong>REMOTE_ADDR:</strong> ${REMOTE_ADDR}</p>"
echo "</div></body></html>"