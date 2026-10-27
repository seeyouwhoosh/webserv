*This project has been created as part of the 42 curriculum by xwu, uelizegi*


# Webserv

*In a nutshell: A custom HTTP/1.1 web server written in C++98.*

## Description

**Webserv** is a 42 project focused on implementing an HTTP web server from scratch in C++98. The goal is to understand how web servers handle network connections, parse HTTP requests, generate responses, serve files, and execute CGI scripts.

The server uses non-blocking sockets and an event-driven architecture to handle multiple client connections without creating a separate thread for each client.

Inspired by NGINX, this project can handle HTTP GET, HEAD, POST, PUT, and DELETE Requests, and can serve static files from a specified root directory or dynamic content using CGI, following the principles of the HTTP/1.1 protocol.


## Features

### HTTP
- HTTP/1.1 request parsing and response generation.
- Support for `GET`, `POST`, and `DELETE` methods.
- HTTP status codes and error handling.
- Request headers, query strings, and message bodies.
- Content-Length handling and MIME type detection.
- Persistent connections and keep-alive handling.

### Networking
- TCP sockets for client connections.
- Non-blocking I/O.
- Event-driven connection management using `epoll`.
- Support for multiple simultaneous clients.
- Partial read and write handling.

### Server configuration
- Multiple servers and listening ports. **We do not support virtual servers**
- Host-based server selection.
- Configurable document roots and index files.
- Location-based routing.
- Allowed HTTP methods per location.
- Configurable maximum request body size.
- Custom error pages.
- Directory listing through autoindex.
- HTTP redirects.

### File handling
- Static file serving.
- Directory and index resolution.
- File uploads to a configurable destination.
- File deletion through `DELETE`, subject to the configured permissions and routing rules. **We do not support folder deletion**
- Content-Type detection based on file extensions.

### CGI
- CGI execution for configured file extensions.
- Passing request information and environment variables to CGI programs.
- Forwarding request bodies to CGI scripts.
- Parsing CGI output and generating HTTP responses.

*And probably more we forgot to add, or less, because we forgot to update the list*

## Instructions
### Requirements

- A Unix-like operating system (Linux recommended because we use `epoll` for event-driven I/O). 
- A C++ compiler supporting C++98.
- `make`.
- CGI interpreters or executables for the scripts you want to run, and your website

### Installation

Clone the repository and compile the project:

```bash
git clone <repository-url>
cd webserv
make
```

The compilation produces the `webserv` executable.

The following commands are available if supported by the project's Makefile:

```bash
make clean   # Remove object files
make fclean  # Remove object files and the executable
make re      # Rebuild the project
```

### Usage

Start the server with the default configuration (`config_files/default.conf`), this will run our test website using the **port 8000**:

```bash
./webserv
```

Or specify a configuration file:

```bash
./webserv config/default.conf
```

The server loads the configuration, initializes its listening sockets, and waits for incoming HTTP requests.

To stop the server, press `Ctrl+C`.

## Configuration

The configuration syntax is inspired by NGINX. Directives define listening addresses, document roots, routing rules, request limits, and other server behavior.

Example configuration:

```nginx
client_max_body_size	5M;
timeout					10s;
cgi_timeout			    5s;

server
{
	listen		8000;
	server_name	default.com;
	root		var/www/html;
	index		index.html;

	error_page	404 404.html;
	error_page	500 500.html;

	location	/ {
		autoindex		on;
		limit_except	GET POST;
		index			index.html;
		upload_store	var/www/html/uploads;
	}

	location	/cgi-bin {
		alias			var/www/cgi-bin;
		cgi				.py /opt/pyenv/shims/python3;
		cgi				.sh /usr/bin/bash;
		cgi				.php /usr/bin/php;
	}

	location	/todelete {
		limit_except	GET DELETE;
		index		    	index.png;
	}
	
	location	/new.html {
		return 301	/old.html;
	}
}
```

This example illustrates the intended configuration structure. Adapt directive names and values to the syntax supported by the version of the parser in this repository.

### Main directives

| Directive | Description | Default value | Directive |
|---|---|---|---|
| `root` | Sets the document root used to resolve requested resources. | inherited or "html" | global, server, location |
| `index` | Specifies the default index file for directories. | inherited or "index.html" | global, server, location |
| `autoindex` | Enables or disables directory listings. | inherited or "off" | global, server, location |
| `client_max_body_size` | Limits the size of incoming request bodies (KB, MB or GB). | inherited of 1MB | global, server, location |
| `timeout` | Defines a timeout for client connection. | inherited or 15s | global, server |
| `cgi_timeout` | Defines a timeout for cgi execution. | inherited or 5s | global, server |
| `error_page` | Configures custom error pages. | inherited or empty | global, server, location |
| `cgi` | Configures CGI handling for supported file extensions. | inherited or empty | global, server, location |
| `server_name` | Defines the server names. | "" | server |
| `listen` | Configures the listening port or address. | "0.0.0.0:8000" | server |
| `location` | Defines rules for a URL path or prefix. | "/" | server |
| `alias` | Defines a replacement for the specified location **This is what the subject refers to "root"** | "" | location |
| `allow_methods` | Restricts the HTTP methods accepted by a location. | "GET POST" | location |
| `return` | Configures HTTP redirects. | empty | location |
| `upload_store` | Specifies the destination for uploaded files. | "" | location |

## Testing

Once the server is running, use a browser or command-line tools to send requests.

### GET

```bash
curl -i http://localhost:8080/
```

Retrieve a static resource:

```bash
curl -i http://localhost:8080/index.html
```

### POST

Send form data:

```bash
curl -i -X POST \
  -d "name=webserv&project=42" \
  http://localhost:8080/
```

### DELETE

```bash
curl -i -X DELETE http://localhost:8080/uploads/file.txt
```

### Custom request headers

```bash
curl -i \
  -H "Host: localhost" \
  http://localhost:8080/
```

### Test with a browser

Open the configured address in your browser:

```text
http://localhost:8080/
```

Make sure the selected port matches the active configuration and is not already in use.

## Project structure

The source code is organized around the main responsibilities of an HTTP server:

- **Configuration parser** — reads and validates the server configuration.
- **Web server** — initializes sockets and manages listening servers and client connections.
- **HTTP message handling** — parses incoming requests and constructs outgoing responses.
- **Request routing** — resolves virtual servers, locations, and filesystem paths.
- **File handling** — serves static resources and handles file operations.
- **CGI handler** — executes configured scripts and processes their output.

Exact filenames and class boundaries may differ depending on the repository's current structure.

## Technical considerations

This project explores several important networking and systems programming concepts:

- TCP sockets and the client-server model.
- Non-blocking I/O and event-driven architectures.
- HTTP request and response formatting.
- Filesystem operations and permissions.
- Process creation and CGI communication.
- Resource management and error handling in C++98.
- Configuration parsing and request routing.

## Resources

### Configuration file
* https://www.getpagespeed.com/check-nginx-config

### General
* https://www.alimnaqvi.com/blog/webserv
* https://nginx101.com/
* https://nginx.org/en/docs/
* https://docs.nginx.com/nginx/admin-guide/web-server/web-server/ 

### Sockets
* https://beej.us/guide/bgnet/html
* https://devarea.com/linux-io-multiplexing-select-vs-poll-vs-epoll/
* https://ncona.com/2019/04/building-a-simple-server-with-cpp/
* https://man7.org/linux/man-pages/man7/epoll.7.html and other man pages


### HTTP requests
* https://httpwg.org/specs/rfc9112.html
* https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status
* https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Methods 

### CGI
* https://www6.uniovi.es/~antonio/ncsa_httpd/cgi/env.html
* http://www.mnuwer.dbasedeveloper.co.uk/dlearn/web/session03.htm

### AI Usage

AI tools were used as a support resource throughout the development of this project. Their use included:

- **Documentation:** Assisting with the writing and organization of this README.
- **Research:** Finding relevant websites, documentation, and online resources.
- **Concept clarification:** Explaining technical terms, concepts, and protocols to support our understanding.
- **Debugging:** Helping identify potential bugs, analyze error messages, and investigate unexpected behavior in the code.
- **Testing:** Assisting in designing test cases, identifying edge cases, and verifying expected server behavior.

AI-generated suggestions were reviewed and evaluated by the team. The implementation, testing, and final decisions remained our responsibility.
