#!/bin/sh

# 1. Read the request body if the method is POST and CONTENT_LENGTH > 0
POST_BODY=""

if [ "$REQUEST_METHOD" = "POST" ] && [ -n "$CONTENT_LENGTH" ] && [ "$CONTENT_LENGTH" -gt 0 ]; then
    POST_BODY=$(dd bs=1 count="$CONTENT_LENGTH" 2>/dev/null)
fi

export POST_BODY

DECODED_BODY=$(python3 -c '
import os
import json
from urllib.parse import parse_qs

body = os.environ.get("POST_BODY", "")

if body.startswith("raw_body="):
    body = parse_qs(body).get("raw_body", [""])[0]

print(body)
')

# 3. Generate HTML
cat <<EOF
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>CGI Shell - Webserv Environment Test</title>
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
        <h1>CGI Environment Test (Shell Script)</h1>
        <p><strong>HTTP Method:</strong> ${REQUEST_METHOD:-GET}</p>
        <p><strong>Executed Script (SCRIPT_NAME):</strong> ${SCRIPT_NAME:-N/A}</p>

        <h3>Received GET Parameters (QUERY_STRING):</h3>
        <pre>${QUERY_STRING:-None}</pre>

        <h3>Request Body Data (POST / STDIN):</h3>
        <pre>${DECODED_BODY:-None}</pre>

        <h3>Main Environment Variables (fill_env):</h3>
        <ul>
            <li><strong>SERVER_SOFTWARE:</strong> ${SERVER_SOFTWARE:-N/A}</li>
            <li><strong>GATEWAY_INTERFACE:</strong> ${GATEWAY_INTERFACE:-N/A}</li>
            <li><strong>SERVER_PROTOCOL:</strong> ${SERVER_PROTOCOL:-N/A}</li>
            <li><strong>SERVER_PORT:</strong> ${SERVER_PORT:-N/A}</li>
            <li><strong>SERVER_NAME:</strong> ${SERVER_NAME:-N/A}</li>
            <li><strong>REQUEST_METHOD:</strong> ${REQUEST_METHOD:-N/A}</li>
            <li><strong>QUERY_STRING:</strong> ${QUERY_STRING:-N/A}</li>
            <li><strong>CONTENT_LENGTH:</strong> ${CONTENT_LENGTH:-0}</li>
            <li><strong>CONTENT_TYPE:</strong> ${CONTENT_TYPE:-N/A}</li>
            <li><strong>PATH_INFO:</strong> ${PATH_INFO:-N/A}</li>
            <li><strong>PATH_TRANSLATED:</strong> ${PATH_TRANSLATED:-N/A}</li>
            <li><strong>REMOTE_ADDR / HOST:</strong> ${REMOTE_ADDR:-N/A}</li>
        </ul>

        <h3>All Process Environment Variables:</h3>
        <pre>$(env)</pre>
    </div>
</body>
</html>
EOF
