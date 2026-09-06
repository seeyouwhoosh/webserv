# Apuntes de webserv

## Notas
- [x] Gestionar las páginas de error (buscar y crear páginas de error)
- [x] Crear funciones para generar fechas
- [x] cambiar `std::vector<t_client>` por `std::map<int, t_client>` porque lo que está dando problemas al parecer es la liberación de eso o algo así
- [x] Gestionar la respuesta para enviarlo non blocking (enviar primero el header y luego el cuerpo a cachos)
- [x] Implementar el autoindex
- [x] Implementar CGI
  - [x] implementar la búsqueda de la carpeta cgi-bin y gestionar eso
  - [x] diferenciar entre los ejecutables y los que son ejecutables por archivos
  - [x] construir la lista de argumentos
- [ ] Parsear el output del cgi
- [ ] Testear el CGI
- [ ] Gestionar la petición del cliente del close connection y keep-alive
- [x] Método GET
- [ ] testear el método DELETE
- [ ] Método POST
  - [ ] Si hay cgi, el marrón se lo come el cgi
  - [ ] Subida de Archivos: Si va dirigido a una ruta donde upload_enable es on, leer el cuerpo de la petición y guardarlo en el directorio especificado en upload_store
  - [ ] Si el tipo es multipart, hay que parsearlo
  - [ ] application/x-www-form-urlencoded se ignora si no hay cgi

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

 * Default: `listen *:80 | *:8000;`
 * Si solo se da el address, se usa el puerto 80
 * Puede hacer varios listens, pero no pueden ser exactamente iguales
 * Formatos que pueden coger:
```conf
    listen 127.0.0.1:8000;
    listen 127.0.0.1; = listen 127.0.0.1:80
    listen 8000; = listen *:8000
    listen *:8000; Nota: gestionar wildcards
    listen localhost:8000; = listen 127.0.0.1:8000
    listen [::]:8000; IPv6; = listen *:8000;
```

 * Posibles adiciones: Soporte de IPv6, `default_server`

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
Default:	index index.html;
Context:	http, server, location
Defines files that will be used as an index. The file name can contain variables. Files are checked in the specified order. The last element of the list can be a file with an absolute path. Example:
```

* Uploading files from the clients to the server is authorized, and storage location
is provided.

No sé, a leer esto: https://www.getpagespeed.com/server-setup/nginx/nginx-upload-module

* Execution of CGI, based on file extension 

fastcgi_pass o no sé

## Nota sobre IPv4

IPv4 addresses are divided into unicast, broadcast, and multicast addresses.  Unicast addresses specify a single interface of a host, broadcast addresses specify all hosts on a network, and multicast addresses address all hosts in a multicast group. Datagrams to broadcast addresses can be sent or received only when
the SO_BROADCAST socket flag is set.

Note that the address and the port are always stored in network byte order.  In particular, this means that you need to call htons(3) on the number that is assigned to a port. All address/port manipulation functions in the standard library work in network byte order.

**IPv4**
```c
struct sockaddr_in {
    short int          sin_family;  // Address family, AF_INET
    unsigned short int sin_port;    // Port number
    struct in_addr     sin_addr;    // Internet address
    unsigned char      sin_zero[8]; // Same size as struct sockaddr
};
// Internet address (a structure for historical reasons)
struct in_addr {
    uint32_t s_addr; // that's a 32-bit int (4 bytes)
};
```

**IPv6**
```c
struct sockaddr_in6 {
    u_int16_t       sin6_family;   // address family, AF_INET6
    u_int16_t       sin6_port;     // port, Network Byte Order
    u_int32_t       sin6_flowinfo; // IPv6 flow information
    struct in6_addr sin6_addr;     // IPv6 address
    u_int32_t       sin6_scope_id; // Scope ID
};

struct in6_addr {
    unsigned char   s6_addr[16];   // IPv6 address
};
```

## Apuntes copiados de beej.us: Jumping from IPv4 to IPv6
1. First of all, try to use getaddrinfo() to get all the struct sockaddr info, instead of packing the structures by hand. This will keep you IP version-agnostic, and will eliminate many of the subsequent steps.

2. Any place that you find you’re hard-coding anything related to the IP version, try to wrap up in a helper function.

3. Change AF_INET to AF_INET6.

4. Change PF_INET to PF_INET6.

5. Change INADDR_ANY assignments to in6addr_any assignments, which are slightly different:

```c
struct sockaddr_in sa;
struct sockaddr_in6 sa6;

sa.sin_addr.s_addr = INADDR_ANY;  // use my IPv4 address
sa6.sin6_addr = in6addr_any; // use my IPv6 address
```

Also, the value IN6ADDR_ANY_INIT can be used as an initializer when the struct in6_addr is declared, like so:

```c
struct in6_addr ia6 = IN6ADDR_ANY_INIT;
```

6. Instead of struct sockaddr_in use struct sockaddr_in6, being sure to add “6” to the fields as appropriate (see structs, above). There is no sin6_zero field.

7. Instead of struct in_addr use struct in6_addr, being sure to add “6” to the fields as appropriate (see structs, above).

8. Instead of inet_aton() or inet_addr(), use inet_pton().

9. Instead of inet_ntoa(), use inet_ntop().

10. Instead of gethostbyname(), use the superior getaddrinfo().

11. Instead of gethostbyaddr(), use the superior getnameinfo() (although gethostbyaddr() can still work with IPv6).

12. INADDR_BROADCAST no longer works. Use IPv6 multicast instead.

## Mini ejemplo
```c++
int	main(int ac, char **av)
{
	/* Create a socket (IPv4, TCP). El tercer argumento es 0 porque suele haber un solo protocolo para para cada tipo (SOCK_STREAM, por ejemplo) */
	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cout << "Error: socket failed\n";
		return -1;
	}

	/*
	Para el tipo de socket que tenemos, en nuestro caso un AF_INET, pasamos una estructura u otra, en este caso, usamos sockaddr_in 
	
	struct sockaddr_in {
	sa_family_t     sin_family;     // AF_INET. Siempre es este.
	in_port_t       sin_port;       //Port number
	struct in_addr  sin_addr;       // IPv4 address
	};

	struct in_addr {
	in_addr_t	s_addr;
	}:

	Para el puerto y la IP, siempre se guardan en el Network Byte Order, así que hay que usar la función htons() "host to network (short)" para convertir un unsigned short a network

	Special addresses
	INADDR_LOOPBACK (127.0.0.1)
	INADDR_ANY (0.0.0.0)
	INADDR_BROADCAST (255.255.255.255)

	*/

	struct sockaddr_in	sockaddr;
	sockaddr.sin_family = AF_INET; // en IPv4 always
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(PORT); //convert host number to network byte

	if (bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0)
	{
		std::cout << "Error: bind to port " << PORT << " failed\n";
		return -1;
	}


	/* Start listening. Hold at most number connections in the queue
	backlog is the maximum number of connections that will be queued before connections start being refused.
	*/

	if (listen(sockfd, 5) < 0)
	{
		std::cout << "Error: listen on socket failed " << errno << "\n";
		return -1;
	}

	std::cout << "Listening for connections on port " << PORT << "\n";

	while (1)
	{
		struct sockaddr_in	clientaddr;
		socklen_t	clientlen = sizeof(clientaddr);
		int	*clientsocket = (int *)malloc(sizeof(int));
	
		/* Grab a connection from the queue
		/* addrlen is now a value-result argument. It expects a pointer to an int that will be the size of addr. After the function is executed, the int refered by addrlen will be set to the size of the peer address.
		*/
		
		if ((*clientsocket = accept(sockfd, (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen)) < 0)
		{
			std::cout << "Error: failed to grab connection " << errno << std::endl;
			continue;
		}
		std::cout << "Connection accepted\n";
		close(*clientsocket);
		std::cout << "Client disconnected\n";
		free(clientsocket);

	}
	close(sockfd);
}
```

## Data encapsulation

A layered model more consistent with Unix might be:

* Application Layer (telnet, ftp, etc.)
* Host-to-Host Transport Layer (TCP, UDP)
* Internet Layer (IP and routing)
* Network Access Layer (Ethernet, wi-fi, or whatever)

A packet is born, the packet is wrapped (“encapsulated”) in a header (and rarely a footer) by the first protocol (say, the TFTP protocol), then the whole thing (TFTP header included) is encapsulated again by the next protocol (say, UDP), then again by the next (IP), then again by the final protocol on the hardware (physical) layer (say, Ethernet).

When another computer receives the packet, the hardware strips the Ethernet header, the kernel strips the IP and UDP headers, the TFTP program strips the TFTP header, and it finally has the data.

## Apuntes para el método GET

```html
    <form action="./submit_page" method="get"> <!-- Un environment para todos los inputs del usuario -->
      <h2>Subscribe to our newsletter</h2>
      <p>
        <label for="name">Name (required):</label> <!-- Básicamente el nombre de la cosa que se pide, con el for, se identifica el nombre de la "variable" pedida (o en otras palabras, el id) -->
        <input type="text" name="name" id="name" required /> <!-- Para exitosamente recibir el input asociado a la variable con el id; explícitamente. EL name al parecer es solo el nombre del elemento input-->
      </p>
      <p>
        <label for="email">Email (required):</label>
        <input type="email" name="email" id="email" required />
      </p>
      <p>
        <button>Sign me up!</button>
      </p>
    </form>
```

### Forma implícita de recoger el input (HTML)
```html
<label>
  I like peas.
  <input type="checkbox" name="peas" />
</label>
```

Dentro del form vemos `action` y `method`, que básicamente, cuando se termina de hacer el request, el servidor enviará un `method action`. Concretamente:

```
/some/url/submit_page?name=Bob&email=bob%40bob.com
```

## CGI

**He empezado por rellenar las variables del cgi, algunas notorias son:**
Ejemplo: `https://ejemplo.com/scripts/app.php/usuarios/123`
* SCRIPT_NAME: `/scripts/app.php`
* PATH_INFO: `/usuarios/123`
* PATH_TRANSLATED: `/var/www/html/usuarios/123`

```
[Cliente] ──► [HTTP Server]
                   │
                   ├── 1. Lee configuración ──► ¿La ruta activa CGI? ──► SÍ
                   ├── 2. Lee Content-Length y recupera el 'body' del POST
                   ├── 3. Prepara Pipe Entrada (stdin) y Pipe Salida (stdout)
                   ├── 4. fork() ──► Process Hijo (execve del script o del intérprete)
                   │                       │
                   │                       ├── Recibe body vía stdin
                   │                       └── Escribe cabeceras/salida en stdout
                   │
                   └── 5. Lee stdout del hijo ──► Formatea respuesta HTTP ──► [Cliente]
```