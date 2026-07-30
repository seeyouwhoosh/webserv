# Apuntes de webserv
## Cosas a hacer
- [ ] Comentarios del config file (#)

## Estructura del archivo de configuración
*Ejemplo:*
```conf
server {
    listen 80; # Listening on port 80 on all IP addresses of the computer

    root /var/www/html; # The directory in which all the website files will live

    index index.html index.htm index.nginx-debian.html; # The file to serve (first one available in this order) when someone requests the "homepage"

    server_name _; # Sets names of a virtual server such as "example.com". This name is matched with the `Host` header in incoming HTTP requests
}
```
* **Directivas simples**: `coso algo;`
* **Directivas de bloque**: `server {}`. Si puede tener más directivas dentro, se llama **contexto**. Algunos son: events, http, server y location

Las directivas colocadas fuera de un contexto de bloque están en el main context.

**Jerarquía de configuración**
```md
main
├── events
└── http
    ├── server
    │   ├── location
    │   └── location
    └── server
```

Según la subject, tenemos que implementar los contextos:
* main (like, el general)
* server
* location

Los directives:

*NOTA: Some directives can appear in multiple contexts, in which case you can override the setting inherited from the parent by including the directive in the child context.*

*  Define all the interface:port pairs on which your server will listen to (defining multiple websites served by your program).

```
server {
    listen 8080;
}

server {
    listen 127.0.0.1:9000;
}
```

https://nginx.org/en/docs/http/ngx_http_core_module.html#listen 

* Set up default error pages.
```
Syntax:	error_page code ... [=[response]] uri;
Default:	—
Context:	http, server, location, if in location
Defines the URI that will be shown for the specified errors. A uri value can contain variables.

Example:

error_page 404             /404.html;
error_page 500 502 503 504 /50x.html;

```

* maximum allowed size for client request bodies
```config
Syntax: 	client_max_body_size size;
Default:	client_max_body_size 1m;
Context:	http, server, location

Sets the maximum allowed size of the client request body. If the size in a request exceeds the configured value, the 413 (Request Entity Too Large) error is returned to the client. Please be aware that browsers cannot correctly display this error. Setting size to 0 disables checking of client request body size.
```

* server_name (virtual hosts)

* List of accepted HTTP methods for the route.
```
Syntax:	    limit_except method ... { ... }
Default:	—
Context:	location
Limits allowed HTTP methods inside a location. The method parameter can be one of the following: GET, HEAD, POST, PUT, DELETE, MKCOL, COPY, MOVE, OPTIONS, PROPFIND, PROPPATCH, LOCK, UNLOCK, or PATCH. Allowing the GET method makes the HEAD method also allowed.
```

* HTTP redirection.
```
Syntax:	return value;
Default:	—
Context:	server
Specifies a value to send to the client. The value can contain text, variables, and their combination.
```

* Directory where the requested file should be located
```
Syntax: 	root path;
Default:	root html;
Context:	http, server, location, if in location
Sets the root directory for requests
```

* Enabling or disabling directory listing.
```
Syntax:	    autoindex on | off;
Default:	autoindex off;
Context:	http, server, location
Enables or disables the directory listing output.
```

* Default file to serve when the requested resource is a directory.
```
Syntax:	index file ...;
Default:	
index index.html;
Context:	http, server, location
Defines files that will be used as an index. The file name can contain variables. Files are checked in the specified order. The last element of the list can be a file with an absolute path. Example:
```

* Uploading files from the clients to the server is authorized, and storage location
is provided.

No sé, a leer esto: https://www.getpagespeed.com/server-setup/nginx/nginx-upload-module

* Execution of CGI, based on file extension 

fastcgi_pass o no sé