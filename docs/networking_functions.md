# sys/socket.h:

## int socket (int domain, int type, int protocol);
Create a new socket of type TYPE in domain DOMAIN, using
protocol PROTOCOL.  If PROTOCOL is zero, one is chosen automatically.
Returns a file descriptor for the new socket, or -1 for errors.

## int setsockopt (int fd, int level, int optname, const void *optval, unsigned it optlen);
Set socket FD's option OPTNAME at protocol level LEVEL
to *OPTVAL (which is OPTLEN bytes long).
Returns 0 on success, -1 for errors.

## int bind(int fd, const struct sockaddr* addr, unsigned int len);
Give the socket FD the local address ADDR (which is LEN bytes long).

## int listen (int fd, int n);
Prepare to accept connections on socket FD.
N connection requests will be queued before further requests are refused.
Returns 0 on success, -1 for errors.

## extern int accept (int fd, struct sockaddr* addr,  unsigned addr_len);
Await a connection on socket FD.
When a connection arrives, open a new socket to communicate with it,
set *ADDR (which is *ADDR_LEN bytes long) to the address of the connecting
peer and *ADDR_LEN to the address's actual length, and return the
new socket's descriptor, or -1 for errors.
This function is a cancellation point and therefore not marked with __THROW.

## ssize_t recv (int fd, void *buf, size_t n, int flags);
Read N bytes into BUF from socket FD.
Returns the number read or -1 for errors.
This function is a cancellation point and therefore not marked with __THROW.

## ssize_t send (int fd, const void *buf, size_t n, int flags);
Send N bytes of BUF to socket FD.  Returns the number sent or -1.
This function is a cancellation point and therefore not marked with __THROW.

# sys/select.h:

## int select(int nfds, ft_set *readfds, fd_set *writefds, struct timeval *timeout);
Check the first NFDS descriptors each in READFDS (if not NULL) for read
readiness, in WRITEFDS (if not NULL) for write readiness, and in EXCEPTFDS
(if not NULL) for exceptional conditions.  If TIMEOUT is not NULL, time out
after waiting the interval specified therein.  Returns the number of ready
descriptors, or -1 for errors.
This function is a cancellation point and therefore not marked with __THROW.

# netinet/in.h:

## uint16 htons(uint16);
Host to Network Short

## uint16 htonl(uint16);
Host to Network Long


# EventManager:

## int EventManager::register_read_callback() ?
## int EventManager::register_write_callback() ?
