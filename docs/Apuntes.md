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