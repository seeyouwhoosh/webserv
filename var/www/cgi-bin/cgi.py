#!/usr/bin/env python3
import sys
import os
import urllib.parse
import json

def main():
    # 1. Required HTTP headers
    print("Content-Type: text/html; charset=utf-8\r")
    print("\r")

    # 2. Retrieve all variables populated by fill_env
    method = os.environ.get("REQUEST_METHOD", "GET")
    query_string = os.environ.get("QUERY_STRING", "")
    content_length = os.environ.get("CONTENT_LENGTH", "0")
    content_type = os.environ.get("CONTENT_TYPE", "Not specified")

    # Standard CGI variables that you mapped
    server_software = os.environ.get("SERVER_SOFTWARE", "N/A")
    gateway_interface = os.environ.get("GATEWAY_INTERFACE", "N/A")
    server_protocol = os.environ.get("SERVER_PROTOCOL", "N/A")
    server_port = os.environ.get("SERVER_PORT", "N/A")
    server_name = os.environ.get("SERVER_NAME", "N/A")

    path_info = os.environ.get("PATH_INFO", "N/A")
    path_translated = os.environ.get("PATH_TRANSLATED", "N/A")
    script_name = os.environ.get("SCRIPT_NAME", "N/A")

    remote_addr = os.environ.get("REMOTE_ADDR", "N/A")

    # 3. Process GET parameters
    get_data = {}
    if query_string:
        parsed_get = urllib.parse.parse_qs(query_string)
        get_data = {
            k: v[0] if len(v) == 1 else v
            for k, v in parsed_get.items()
        }

    # 4. Process POST / request body received through stdin
    post_data = "None"
    if method == "POST":
        try:
            length = int(content_length) if content_length else 0
            if length > 0:
                post_body = sys.stdin.read(length)

                if "application/json" in content_type:
                    post_data = json.loads(post_body)
                else:
                    # Try parsing as form-urlencoded; if that fails,
                    # display the raw text received.
                    try:
                        parsed_post = urllib.parse.parse_qs(post_body)
                        post_data = {
                            k: v[0] if len(v) == 1 else v
                            for k, v in parsed_post.items()
                        }

                        if not post_data:
                            # If it was not in key=value format,
                            # display the raw string.
                            post_data = post_body
                    except Exception:
                        post_data = post_body
            else:
                post_data = "Empty body (CONTENT_LENGTH is 0)"

        except Exception as e:
            post_data = f"Error reading POST body: {str(e)}"

    # 5. Filter additional HTTP headers (those starting with HTTP_)
    http_headers = {
        k: v for k, v in os.environ.items()
        if k.startswith("HTTP_")
    }

    # 6. Generate HTML with the page layout
    print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>CGI Python - Webserv Environment Test</title>
    <style>
        body {{ font-family: Arial, sans-serif; background: #0f172a; color: #f8fafc; padding: 20px; }}
        .box {{ background: #1e293b; padding: 20px; border-radius: 8px; border: 1px solid #334155; max-width: 900px; margin: auto; }}
        h1 {{ color: #38bdf8; }}
        h3 {{ color: #38bdf8; border-bottom: 1px solid #334155; padding-bottom: 5px; margin-top: 25px; }}
        pre {{ background: #0f172a; padding: 10px; border-radius: 4px; color: #22c55e; overflow-x: auto; }}
        ul {{ line-height: 1.6; }}
    </style>
</head>
<body>
    <div class="box">
        <h1>CGI Environment Test (Python)!</h1>
        <p><strong>HTTP Method:</strong> {method}</p>
        <p><strong>Executed Script (SCRIPT_NAME):</strong> {script_name}</p>

        <h3>Received GET Parameters:</h3>
        <pre>{json.dumps(get_data, indent=4, ensure_ascii=False) if get_data else "None"}</pre>

        <h3>Request Body Data (POST / STDIN):</h3>
        <pre>{json.dumps(post_data, indent=4, ensure_ascii=False) if isinstance(post_data, (dict, list)) else post_data}</pre>

        <h3>Main Environment Variables (fill_env):</h3>
        <ul>
            <li><strong>SERVER_SOFTWARE:</strong> {server_software}</li>
            <li><strong>GATEWAY_INTERFACE:</strong> {gateway_interface}</li>
            <li><strong>SERVER_PROTOCOL:</strong> {server_protocol}</li>
            <li><strong>SERVER_PORT:</strong> {server_port}</li>
            <li><strong>SERVER_NAME:</strong> {server_name}</li>
            <li><strong>REQUEST_METHOD:</strong> {method}</li>
            <li><strong>QUERY_STRING:</strong> {query_string if query_string else "N/A"}</li>
            <li><strong>CONTENT_LENGTH:</strong> {content_length}</li>
            <li><strong>CONTENT_TYPE:</strong> {content_type}</li>
            <li><strong>PATH_INFO:</strong> {path_info}</li>
            <li><strong>PATH_TRANSLATED:</strong> {path_translated}</li>
            <li><strong>REMOTE_ADDR / HOST:</strong> {remote_addr}</li>
        </ul>
    </div>
</body>
</html>""")

if __name__ == "__main__":
    main()
