# Funciones de Unix/Linux y Sockets

## Procesos

### `#include <unistd.h>`
```c
int execve(const char *pathname,
           char *const argv[],
           char *const envp[]);
```

Reemplaza el proceso actual por otro programa.

- No crea un proceso nuevo.
- Si tiene éxito, nunca retorna.
- Devuelve `-1` si ocurre un error.

```c
pid_t fork(void);
```

Crea un proceso hijo duplicando el proceso actual.

Valor de retorno:

- `0` → proceso hijo.
- `>0` → PID del hijo (proceso padre).
- `-1` → error.

### `#include <sys/wait.h>`

```
pid_t waitpid(pid_t pid, int *status, int options);
```

Espera a que termine un proceso hijo.

### `#include <signal.h>`

```c
int kill(pid_t pid, int sig);
```

Envía una señal a un proceso.

```c
void (*signal(int signum, void (*handler)(int)))(int);
```

Instala un manejador para una señal.

# Pipes y descriptores

### `#include <unistd.h>`

```c
int pipe(int fd[2]);
```

Crea un pipe.

- `fd[0]` → lectura
- `fd[1]` → escritura

```c
int dup(int oldfd);
```

Duplica un descriptor de archivo.

```c
int dup2(int oldfd, int newfd);
```

Hace que `newfd` apunte al mismo recurso que `oldfd`.


# Manejo de errores

### `#include <string.h>`

```c
char *strerror(int errnum);
```
Convierte un código de error (`errno`) en una cadena descriptiva.

### `#include <netdb.h>`

```c
const char *gai_strerror(int errcode);
```
Convierte un código de error devuelto por `getaddrinfo()` en texto.


### `#include <errno.h>`

Variable global que contiene el código del último error producido por una llamada al sistema.

# Conversión de orden de bytes

### `#include <arpa/inet.h>`


```c
uint16_t htons(uint16_t hostshort);
```
Convierte un entero de 16 bits de formato Host a Network.

```c
uint32_t htonl(uint32_t hostlong);
```
Convierte un entero de 32 bits de Host a Network.

```c
uint16_t ntohs(uint16_t netshort);
```

Convierte un entero de 16 bits de Network a Host.

```c
uint32_t ntohl(uint32_t netlong);
```

Convierte un entero de 32 bits de Network a Host.

# Multiplexación

### `#include <sys/select.h>`

```c
int select(int nfds,
           fd_set *readfds,
           fd_set *writefds,
           fd_set *exceptfds,
           struct timeval *timeout);
```

Espera hasta que uno o varios descriptores estén listos para leer, escribir o tengan excepciones.

### `#include <poll.h>`

```c
int poll(struct pollfd *fds,
         nfds_t nfds,
         int timeout);
```

Alternativa moderna a `select()`.

# epoll (Linux)

### `#include <sys/epoll.h>`

```c
int epoll_create(int size);
```

Crea una instancia de epoll.

```c
int epoll_ctl(int epfd,
              int op,
              int fd,
              struct epoll_event *event);
```

Añade, modifica o elimina descriptores monitorizados.

```c
int epoll_wait(int epfd,
               struct epoll_event *events,
               int maxevents,
               int timeout);
```

Espera eventos.

# kqueue (BSD/macOS)

### `#include <sys/event.h>`

```c
int kqueue(void);
```

Crea una cola de eventos.


```c
int kevent(int kq,
           const struct kevent *changelist,
           int nchanges,
           struct kevent *eventlist,
           int nevents,
           const struct timespec *timeout);
```

Gestiona y espera eventos.

# Sockets

### `#include <sys/socket.h>`

```c
int socket(int domain,
           int type,
           int protocol);
```

Crea un socket.

```c
int socketpair(int domain,
               int type,
               int protocol,
               int sv[2]);
```

Crea dos sockets conectados entre sí.

```c
int bind(int sockfd,
         const struct sockaddr *addr,
         socklen_t addrlen);
```

Asocia una dirección (IP y puerto) al socket.

```c
int listen(int sockfd,
           int backlog);
```

Pone el socket en modo escucha.


```c
int accept(int sockfd,
           struct sockaddr *addr,
           socklen_t *addrlen);
```

Acepta una conexión entrante.


```c
int connect(int sockfd,
            const struct sockaddr *addr,
            socklen_t addrlen);
```

Conecta con un servidor.

```c
ssize_t send(int sockfd,
             const void *buf,
             size_t len,
             int flags);
```

Envía datos por un socket.


```c
ssize_t recv(int sockfd,
             void *buf,
             size_t len,
             int flags);
```

Recibe datos desde un socket.

```c
int setsockopt(int sockfd,
               int level,
               int optname,
               const void *optval,
               socklen_t optlen);
```

Configura opciones del socket.


```c
int getsockname(int sockfd,
                struct sockaddr *addr,
                socklen_t *addrlen);
```

Obtiene la dirección local asociada al socket.

# Resolución de nombres

### `#include <netdb.h>`

```c
int getaddrinfo(const char *node,
                const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);
```

Resuelve un nombre de host (o dirección IP) y un servicio (o puerto) en una lista de direcciones utilizables.


```c
void freeaddrinfo(struct addrinfo *res);
```

Libera la memoria reservada por `getaddrinfo()`.

```c
struct protoent *getprotobyname(const char *name);
```

Obtiene información sobre un protocolo (por ejemplo, `"tcp"` o `"udp"`).

# Directorios

### `#include <unistd.h>`

```c
int chdir(const char *path);
```

Cambia el directorio de trabajo.

```c
DIR *opendir(const char *name);
```

Abre un directorio.

### `#include <dirent.h>`

```c
struct dirent *readdir(DIR *dirp);
```

Lee la siguiente entrada de un directorio.

```c
int closedir(DIR *dirp);
```

Cierra un directorio.

# Archivos

### `#include <fcntl.h>`

```c
int open(const char *pathname,
         int flags,
         mode_t mode);
```

Abre o crea un archivo.

### `#include <unistd.h>`

```c
int close(int fd);
```

Cierra un descriptor.

```c
ssize_t read(int fd,
             void *buf,
             size_t count);
```

Lee datos desde un descriptor.

```c
ssize_t write(int fd,
              const void *buf,
              size_t count);
```

Escribe datos en un descriptor.

```c
int access(const char *pathname,
           int mode);
```

Comprueba si existe un archivo y si se tienen determinados permisos sobre él.

### `#include <sys/stat.h>`

```c
int stat(const char *pathname,
         struct stat *statbuf);
```

Obtiene información sobre un archivo (permisos, tamaño, fechas, tipo, etc.).

# Control de descriptores

### `#include <fcntl.h>`

```c
int fcntl(int fd, int cmd, ...);
```

Permite modificar o consultar propiedades de un descriptor de archivo (flags, bloqueo, modo no bloqueante, duplicación, etc.).
