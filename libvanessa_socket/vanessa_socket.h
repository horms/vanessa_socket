/**********************************************************************
 * vanessa_socket.h                                       November 1999
 * Simon Horman                                      horms@verge.net.au
 *
 * Open a tcp socket to a server (we are the client)
 *
 * vanessa_socket
 * Library to simplify handling of TCP sockets
 * Copyright (C) 1999-2008  Simon Horman <horms@verge.net.au>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA
 *
 **********************************************************************/

#ifndef TCP_PIPE_NYM
#define TCP_PIPE_NYM

#include <ctype.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <syslog.h>

#include <vanessa_logger.h>

#ifndef SOMAXCONN
#define SOMAXCONN 5
#endif

/* Needed for Solaris */
#ifdef sun
#define timeradd(a, b, result)                                                \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;                          \
    if ((result)->tv_usec >= 1000000)                                         \
      {                                                                       \
        ++(result)->tv_sec;                                                   \
        (result)->tv_usec -= 1000000;                                         \
      }                                                                       \
  } while (0)

#define timersub(a, b, result)                                                \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                          \
    if ((result)->tv_usec < 0) {                                              \
      --(result)->tv_sec;                                                     \
      (result)->tv_usec += 1000000;                                           \
    }                                                                         \
  } while (0)
#endif				/* sun */

typedef unsigned int vanessa_socket_flag_t;

#define VANESSA_SOCKET_NONE            0x00000000
#define VANESSA_SOCKET_NO_LOOKUP       0x00000001
#define VANESSA_SOCKET_NO_FROM         0x00000002
#define VANESSA_SOCKET_NO_FORK         0x00000004
#define VANESSA_SOCKET_TCP_KEEPALIVE   0x00000008

#define VANESSA_SOCKET_PROTO_MASK      0x0000ff00
#define __VANESSA_SOCKET_PROTO(_proto)   ((_proto&0xff)<<8)
#define VANESSA_SOCKET_PROTO_TCP       __VANESSA_SOCKET_PROTO(IPPROTO_TCP)
#define VANESSA_SOCKET_PROTO_UDP       __VANESSA_SOCKET_PROTO(IPPROTO_UDP)
#define VANESSA_SOCKET_PROTO_STR_TCP   "tcp"
#define VANESSA_SOCKET_PROTO_STR_UDP   "udp"

#ifndef INPORT_ANY
#define INPORT_ANY     ((int)0)
#endif



/**********************************************************************
 * vanessa_socket_client_open_sockaddr_in
 * Open a socket connection as a client
 * pre: to: sockaddr structure specifying address and port to connect 
 *          to.
 *      flag: If VANESSA_SOCKET_NO_LOOKUP then no host and port lookups
 *            will be performed
 * post: socket is opened
 * return: open socket
 *         -1 on error
 **********************************************************************/

int vanessa_socket_client_open_sockaddr_in(struct sockaddr_in to,
					   const vanessa_socket_flag_t
					   flag);


/**********************************************************************
 * vanessa_socket_client_open
 * Open a socket connection as a client
 * pre: host: hostname or ipaddress to open socket to
 *      port: name or number to open
 *      flag: If VANESSA_SOCKET_NO_LOOKUP then no host and port lookups
 *            will be performed
 * post: socket is opened
 * return: open socket
 *         -1 on error
 **********************************************************************/

int vanessa_socket_client_open(const char *host,
			       const char *port,
			       const vanessa_socket_flag_t flag);


/**********************************************************************
 * vanessa_socket_client_open_src_sockaddr_in
 * Open a socket connection as a client
 * pre: from: sockaddr structure specifying address and port to connect
 *            from. 
 *            If from.sin_addr.s_addr==INADDR_ANY then the operating 
 *            system will select an appropriate source address.
 *            If from.sin_port==INPORT_ANY then the operating system 
 *            will select an appropriate source address.
 *      to: sockaddr structure specifying address and port to connect 
 *          to.
 *      flag: Logical or of VANESSA_SOCKET_NO_LOOKUP and 
 *            VANESSA_SOCKET_NO_FROM
 *            If flag&VANESSA_SOCKET_NO_LOOKUP then no host and port 
 *            lookups will be performed 
 *            If flag&VANESSA_SOCKET_NO_FROM then the from parameter 
 *            will not be used and the operating system will select a 
 *            source address and port
 * post: socket is opened
 * return: open socket
 *         -1 on error
 **********************************************************************/

int vanessa_socket_client_open_src_sockaddr_in(struct sockaddr_in from,
					       struct sockaddr_in to,
					       const vanessa_socket_flag_t
					       flag);


/**********************************************************************
 * vanessa_socket_client_src_open
 * Open a socket connection as a client
 * pre: src_host: hostname or ipaddress to open socket to
 *                If NULL then the operating system will select
 *                an appropriate source address.
 *      src_port: name or number to open
 *                If NULL then the operating system will select
 *                an appropriate source port.
 *      dst_host: hostname or ipaddress to open socket to
 *      dst_port: name or number to open
 *      flag: Logical or of VANESSA_SOCKET_NO_LOOKUP and 
 *            VANESSA_SOCKET_NO_FROM
 *            If flag&VANESSA_SOCKET_NO_LOOKUP then no host and port 
 *            lookups will be performed 
 *            If flag&VANESSA_SOCKET_NO_FROM then the from parameter 
 *            will not be used and the operating system will select a 
 *            source address and port
 * post: socket is opened
 * return: open socket
 *         -1 on error
 **********************************************************************/

int vanessa_socket_client_src_open(const char *src_host,
				   const char *src_port,
				   const char *dst_host,
				   const char *dst_port,
				   const vanessa_socket_flag_t flag);


/**********************************************************************
 * vanessa_socket_str_is_digit
 * Test if a null terminated string is composed entirely of digits (0-9)
 * pre: String
 * return: 1 if string contains only digits and null terminator
 *         0 if string is NULL
 *         0 otherwise
 **********************************************************************/


int vanessa_socket_str_is_digit(const char *str);



/**********************************************************************
 * Notes on read_func and write_func
 *
 * The vanessa_socket_pipe_*func() group of functions take 
 * read_func, write_func and data arguments. read_func and write_func
 * are intended to allow the application to provide its own low
 * level reading and writing routines, for instance to allow
 * reading and writing to SSL/TLS. The data argument is intended
 * to be an opaque data type which may be used by read_func and
 * write_func. These functions should behave as follows.
 * vanessa_socket_pipe_fd_read and vanessa_socket_pipe_fd_write
 * are examples of implementations of these functions that
 * allow reading and writing using read(2) and write(2).
 *
 * read_func
 * pre: fd: file descriptor to read bytes from
 *      buf: buffer to read bytes into
 *      count: maximum number of bytes to read
 *      data: opaque data that may be passed from the application and
 *            used by read_func
 * post: A maximum of count bytes are read into buf.
 *       If an error occurs then it may be logged by this function.
 * return: Number of bytes read (may be zero)
 *         -1 on error
 *
 * write_func
 * pre: fd: file descriptor to write bytes to
 *      buf: buffer to write bytes from
 *      count: maximum number of bytes to write
 *      data: opaque data that may be passed from the application and
 *            used by write_func
 * post: A maximum of count bytes are written from buf
 *       If an error occurs then it may be logged by this function.
 *       installed logger. See vanessa_logger_set().
 * return: Number of bytes read (may be zero)
 *         -1 on error
 **********************************************************************/



/**********************************************************************
 * vanessa_socket_pipe_fd_read
 * Read bytes from fd
 * Intended to be passed to vanessa_socket_pipe_*func()
 * pre: fd: file descriptor to read bytes from
 *      buf: buffer to read bytes into
 *      count: maximum number of bytes to read
 *      data: not used
 * post: A maximum of count bytes are read into buf from fd using read(2)
 *       If an error occurs the errno is read and logged using the
 *       installed logger. See vanessa_logger_set().
 * return: Number of bytes read (may be zero)
 *         -1 on error
 **********************************************************************/

ssize_t vanessa_socket_pipe_fd_read(int fd, void *buf, size_t count,
				    void *data);


/**********************************************************************
 * vanessa_socket_pipe_fd_write
 * Write bytes to fd
 * Intended to be passed to vanessa_socket_pipe_*func()
 * pre: fd: file descriptor to write bytes to
 *      buf: buffer to write bytes from
 *      count: maximum number of bytes to write
 *      data: not used
 * post: A maximum of count bytes are written to fd from buf using write(2)
 *       If an error occurs the errno is read and logged using the
 *       installed logger. See vanessa_logger_set().
 * return: Number of bytes read (may be zero)
 *         -1 on error
 **********************************************************************/

ssize_t vanessa_socket_pipe_fd_write(int fd, const void *buf, size_t count,
				     void *data);


/**********************************************************************
 * vanessa_socket_pipe_func
 * pipe data between two pairs of file descriptors until there is an 
 * error, timeout or one or both the file descriptors are closed.
 * pre: rfd_a: one of the read file descriptors
 *      wfd_a: one of the write file descriptors
 *      rfd_b: the other read file descriptor
 *      wfd_b: the other write file descriptor
 *      buffer:   allocated buffer to read data into
 *      buffer_length: size of buffer in bytes
 *      idle_timeout:  timeout in seconds to wait for input
 *                     timeout of 0 = infinite timeout
 *      return_a_read_bytes: Pointer to size_t where number
 *                           of bytes read from a will be recorded.
 *                           Note that this may wrap
 *      return_b_read_bytes: Pointer to size_t where number
 *                           of bytes read from b will be recorded.
 *                           Note that this may wrap
 *      read_func: Function to use for low level reading.
 *                 If NULL, a simple wrapper around read(2) is used
 *      write_func: Function to use for low level writing.
 *                 If NULL, a simple wrapper around write(2) is used
 *                 If NULL, a simple wrapper around read(2) is used
 *      select_func: Function to use for select
 *                 If NULL, a simple wrapper around select(2) is used
 *      data: opaque data passed to read_func, write_func and select_func
 * post: bytes are read from io_a and written to io_b and vice versa
 * return: -1 on error
 *         1 on idle timeout
 *         0 otherwise (one of io_a or io_b closes gracefully)
 **********************************************************************/

int vanessa_socket_pipe_func(int rfd_a,
			     int wfd_a,
			     int rfd_b,
			     int wfd_b,
			     char *buffer,
			     int buffer_length,
			     int idle_timeout,
			     size_t *return_a_read_bytes,
			     size_t *return_b_read_bytes,
			     ssize_t(*read_func) (int fd, void *buf,
						  size_t count,
						  void *data),
			     ssize_t(*write_func) (int fd, const void *buf,
						   size_t count,
						   void *data),
			     int(*select_func) (int n, fd_set *readfds,
				                fd_set *writefds,
						fd_set *exceptfds,
						struct timeval *timeout,
						void *data),
			     void *data);


/**********************************************************************
 * vanessa_socket_pipe
 * pipe data between two pairs of file descriptors until there is an 
 * error, timeout or one or both the file descriptors are closed.
 * pre: rfd_a: one of the read file descriptors
 *      wfd_a: one of the write file descriptors
 *      rfd_b: the other read file descriptor
 *      wfd_b: the other write file descriptor
 *      buffer:   allocated buffer to read data into
 *      buffer_length: size of buffer in bytes
 *      idle_timeout:  timeout in seconds to wait for input
 *                     timeout of 0 = infinite timeout
 *      return_a_read_bytes: Pointer to int where number
 *                           of bytes read from a will be recorded.
 *      return_b_read_bytes: Pointer to int where number
 *                           of bytes read from b will be recorded.
 * post: bytes are read from io_a and written to io_b and vice versa
 * return: -1 on error
 *         1 on idle timeout
 *         0 otherwise (one of io_a or io_b closes gracefully)
 **********************************************************************/

#define vanessa_socket_pipe( \
  rfd_a,  \
  wfd_a,  \
  rfd_b,  \
  wfd_b,  \
  buffer, \
  buffer_length, \
  idle_timeout, \
  return_a_read_bytes, \
  return_b_read_bytes \
) \
  vanessa_socket_pipe_func( \
    rfd_a,  \
    wfd_a,  \
    rfd_b,  \
    wfd_b,  \
    buffer, \
    buffer_length,  \
    idle_timeout,  \
    return_a_read_bytes, \
    return_b_read_bytes,  \
    NULL, \
    NULL, \
    NULL, \
    NULL \
  )


/**********************************************************************
 * vanessa_socket_pipe_read_write_func
 * Read data from one file io_t and write to another
 * pre: rfd: file descriptor to read from
 *      wfd: file descriptor to write to
 *      buffer: allocated buffer to store read data in
 *      buffer_length: size of the buffer
 *      read_func: function to use for low level reading
 *                 If NULL, a simple wrapper around read(2) is used
 *      write_func: function to use for low level writing
 *                 If NULL, a simple wrapper around write(2) is used
 *      data: opaque data passed to read_func and write_func
 * post: at most buffer_length bytes are read from in_fd and written 
 *       to out_fd. 
 * return: bytes read on success
 *         0 on EOF
 *         -1 on error
 **********************************************************************/

ssize_t vanessa_socket_pipe_read_write_func(int rfd,
					int wfd,
					char *buffer,
					int buffer_length,
					ssize_t(*read_func) (int fd,
							     void *buf,
							     size_t count,
							     void *data),
					ssize_t(*write_func) (int fd,
							      const void
							      *buf,
							      size_t count,
							      void *data),
					void *data);


/**********************************************************************
 * vanessa_socket_pipe_read_write
 * Read data from one file io_t and write to another
 * pre: rfd: file descriptor to read from
 *      wfd: file descriptor to write to
 *      buffer: allocated buffer to store read data in
 *      buffer_length: size of the buffer
 * post: at most buffer_length bytes are read from in_fd and written 
 *       to out_fd. 
 * return: bytes read on success
 *         0 on EOF
 *         -1 on error
 **********************************************************************/

#define vanessa_socket_pipe_read_write(rfd, wfd, buffer, buffer_length) \
  vanessa_socket_pipe_read_write_func(\
    rfd, wfd, \
    buffer, buffer_length, \
    vanessa_socket_pipe_fd_read, vanessa_socket_pipe_fd_write, \
    NULL \
  )


/**********************************************************************
 * vanessa_socket_pipe_write_bytes_func
 * write a n bytes of a buffer to fd
 * Pre: fd: file descriptor to write to
 *      buffer: buffer to write
 *      n: number or bytes to write
 *      write_func: function to use for low level writing
 *                 If NULL, a simple wrapper around write(2) is used
 *      fd_data: opaque data relating to fd, to pass to write_func
 * Return: -1 on error
 *         0 otherwise
 **********************************************************************/

int vanessa_socket_pipe_write_bytes_func(int fd,
					 const char *buffer,
					 const ssize_t n,
					 ssize_t(*write_func) (int fd,
							       const void
							       *buf,
							       size_t
							       count,
							       void *data),
					 void *fd_data);


/**********************************************************************
 * vanessa_socket_pipe_write_bytes
 * write a n bytes of a buffer to fd
 * Pre: fd: file descriptor to write to
 *      buffer: buffer to write
 *      n: number or bytes to write
 * Return: -1 on error
 *         0 otherwise
 **********************************************************************/

#define vanessa_socket_pipe_write_bytes(fd, buffer, n) \
  vanessa_socket_pipe_write_bytes_func(fd, buffer, n, \
    vanessa_socket_pipe_fd_write, NULL)


/**********************************************************************
 * vanessa_socket_server_bind
 * Open a socket and bind it to a port and address
 * pre: port: port to listen to, an ASCII representation of a number
 *            or an entry from /etc/services
 *      interface_address: If NULL bind to 0.0.0.0, else
 *                         bind to interface(es) with this address.
 *      flag: If VANESSA_SOCKET_NO_LOOKUP then no host and port lookups
 *            will be performed
 * post: Bound socket is returned
 * return: socket
 *         -1 on error
 **********************************************************************/

int vanessa_socket_server_bind(const char *port,
				     const char *interface_address,
				     vanessa_socket_flag_t flag);


/**********************************************************************
 * vanessa_socket_server_bindv
 * Open sockets and bind them to a ports and address
 * pre: fromv: NULL terminated pointer of interface addresses and ports
 *             [addr1, port1, addr2, port1, ..., addrN, portN, NULL]
 *             If you want to bind to "0.0.0.0" specify it litreally, rather
 *             than as NULL as works with vanessa_socket_server_bind.
 *             Note: binding to "0.0.0.0" and other addresses may not work
 *             according to your operating system.
 *      flag: passed to vanessa_socket_server_bind
 * return: -1 terminated pointer of bound sockets
 *         To close the sockets and free, call vanessa_socket_closev();
 *         NULL on error
 **********************************************************************/

int *
vanessa_socket_server_bindv(const const char **fromv,
                            vanessa_socket_flag_t flag);


/**********************************************************************
 * vanessa_socket_server_bind_sockaddr_in
 * Open a socket and bind it to a port and address
 * pre: from: sockaddr_in to bind to
 *      flag: ignored
 * post: Bound socket
 * return: socket
 *         -1 on error
 **********************************************************************/

int vanessa_socket_server_bind_sockaddr_in(struct sockaddr_in from,
					   vanessa_socket_flag_t flag);


/**********************************************************************
 * vanessa_socket_server_bind_sockaddr_inv
 * Open sockets and bind them to a ports and address
 * pre: fromv: non-terminated pointer of sockaddr_in to bind to
 *             [sockaddr1, sockaddr2,..., sockaddrN]
 *             N.B: This would be NULL terminated, as per
 *                  vanessa_socket_server_bind_sockaddrv, 
 *                  but you can't have a NULL struct sockaddr_in
 *      nfrom: Number of elements in fromv
 *      flag: ignored
 * post: Bound socket
 * return: -1 terminated pointer of bound sockets
 *         To close the sockets and free, call vanessa_socket_closev();
 *         NULL on error
 **********************************************************************/

int *
vanessa_socket_server_bind_sockaddr_inv(struct sockaddr_in *fromv, 
					size_t nfrom, 
					vanessa_socket_flag_t flag);


/**********************************************************************
 * vanessa_socket_closev
 * Close sockets
 * pre: fromv: -1 terminated pointer of sockets
 *             [sockaddr1, sockaddr2,..., sockaddrN, NULL]
 *      flag: ignored
 * post: Sockets are closed
 *       sockv is freed
 * return: The first non-zero return status returned by an call to close()
 *         0 if all close() calls were successful
 **********************************************************************/

int
vanessa_socket_closev(int *sockv);


/**********************************************************************
 * vanessa_socket_server_accept
 * Accept connections on a bound socket.
 * vanessa_socket_server_bind or vanessa_socket_server_bind_sockaddr_in
 * may be used to open the bound socket.
 * When one is received fork
 * In the Child: close listen_socket and
 *               return the file descriptor that is 
 *               a socket connection to the client
 * In the Server: close the socket to the client and loop
 * pre: listen_socket: socket to listen for connection on
 *      maximum_connections: maximum number of active connections
 *                           to handle. If 0 then an number of connections 
 *                           is unlimited.
 *                           Not used if flag is VANESSA_SOCKET_NO_FORK
 *      return_from: pointer to a struct sockaddr where the 
 *                   connecting client's IP address will
 *                   be placed. Ignored if NULL
 *      return_to: pointer to a struct sockaddr where the IP address the 
 *                 server accepted the connection on will be placed.
 *                 Ignored if NULL
 *      flag: If VANESSA_SOCKET_NO_FORK then the process does not fork
 *            when a connection is recieved.
 * post: Client sockets are returned in child processes
 *       In the parent process the function doesn't exit, other 
 *       than on error.
 *       if return_from is non-null, it is seeded with cleints address
 * return: client socket, if connection is accepted.
 *         -1 on error
 **********************************************************************/

int vanessa_socket_server_accept(int listen_socket,
				      const unsigned int maximum_connections,
				      struct sockaddr *return_from, 
				      struct sockaddr *return_to,
				      vanessa_socket_flag_t flag);


/**********************************************************************
 * vanessa_socket_server_acceptv
 * Accept connections on a bound socket.
 * vanessa_socket_server_bind or vanessa_socket_server_bind_sockaddr_in
 * may be used to open the bound socket.
 * When one is received fork
 * In the Child: close each socket in listen_socketv and
 *               return the file descriptor that is 
 *               a socket connection to the client
 * In the Server: close the socket to the client and loop
 * pre: port: port to listen to, an ASCII representation of a number
 *            or an entry from /etc/services
 *      listen_socketc: -1 terminated pointer to sockets to listen on
 *      maximum_connections: maximum number of active connections
 *                           to handle. If 0 then an number of connections 
 *                           is unlimited.
 *                           Not used if flag is VANESSA_SOCKET_NO_FORK
 *      return_from: pointer to a struct sockaddr where the 
 *                   connecting client's IP address will
 *                   be placed. Ignored if NULL
 *      return_to: pointer to a struct sockaddr where the IP address the 
 *                 server accepted the connection on will be placed.
 *                 Ignored if NULL
 *      flag: If VANESSA_SOCKET_NO_FORK then the process does not fork
 *            when a connection is recieved.
 * post: Client sockets are returned in child processes
 *       In the parent process the function doesn't exit, other 
 *       than on error.
 *       if return_from is non-null, it is seeded with cleints address
 * return: client socket, if connection is accepted.
 *         -1 on error
 **********************************************************************/

int 
vanessa_socket_server_acceptv(int *listen_socketv,
				      const unsigned int maximum_connections,
				      struct sockaddr *return_from, 
				      struct sockaddr *return_to,
				      vanessa_socket_flag_t flag);


/**********************************************************************
 * vanessa_socket_server_connect
 * Listen on a tcp port for incoming client connections 
 * When one is received fork
 * In the Child: close the listening file descriptor
 *               return the file descriptor that is 
 *               a socket connection to the client
 * In the Server: close the socket to the client and loop
 * pre: port: port to listen to, an ASCII representation of a number
 *            or an entry from /etc/services
 *      interface_address: If NULL bind to all interfaces, else
 *                         bind to interface(es) with this address.
 *      maximum_connections: maximum number of active connections to
 *                           handle. If 0 then an number of connections 
 *                           is unlimited.
 *      return_from: pointer to a struct_in addr where the 
 *                   connecting client's IP address will
 *                   be placed. Ignored if NULL
 *      return_to: pointer to a in addr where the IP address the 
 *                 server accepted the connection on will be placed.
 *                 Ignored if NULL
 *      flag: If VANESSA_SOCKET_NO_LOOKUP then no host and port lookups
 *            will be performed
 * post: Client sockets are returned in child processes
 *       In the parent process the function doesn't exit, other 
 *       than on error.
 *       if return_from is non-null, it is seeded with clients address
 * return: open client socket, if connection is accepted.
 *         -1 on error
 **********************************************************************/

int vanessa_socket_server_connect(const char *port,
				  const char *interface_address,
				  const unsigned int maximum_connections,
				  struct sockaddr *return_from,
				  struct sockaddr *return_to,
				  vanessa_socket_flag_t flag);


/**********************************************************************
 * vanessa_socket_server_connectv
 * Listen on a tcp port for incoming client connections 
 * When one is received fork
 * In the Child: close the listening file descriptor
 *               return the file descriptor that is 
 *               a socket connection to the client
 * In the Server: close the socket to the client and loop
 * pre: fromv: NULL terminated pointer of interface addresses and ports
 *             [addr1, port1, addr2, port1, ..., addrN, portN, NULL]
 *             If you want to bind to "0.0.0.0" specify it litreally, rather
 *             than as NULL as works with vanessa_socket_server_connect.
 *             Note: binding to "0.0.0.0" and other addresses may not work
 *             according to your operating system.
 *      maximum_connections: maximum number of active connections to
 *                           handle. If 0 then an number of connections 
 *                           is unlimited.
 *      return_from: pointer to a struct_in addr where the 
 *                   connecting client's IP address will
 *                   be placed. Ignored if NULL
 *      return_to: pointer to a in addr where the IP address the 
 *                 server accepted the connection on will be placed.
 *                 Ignored if NULL
 *      flag: If VANESSA_SOCKET_NO_LOOKUP then no host and port lookups
 *            will be performed
 * post: Client sockets are returned in child processes
 *       In the parent process the function doesn't exit, other 
 *       than on error.
 *       if return_from is non-null, it is seeded with clients address
 * return: open client socket, if connection is accepted.
 *         -1 on error
 **********************************************************************/

int 
vanessa_socket_server_connectv(const const char **from,
			       const unsigned int maximum_connections,
			       struct sockaddr *return_from,
			       struct sockaddr *return_to,
			       vanessa_socket_flag_t flag);


/**********************************************************************
 * vanessa_socket_server_reaper
 * A signal handler that waits for SIGCHLD and runs wait3 to free
 * the resources of any exited children. This stops zombie processes
 * from hanging around.
 * pre: SIGCHLD is received by the process
 * post: Resources of any exited children are freed
 * return: 0
 **********************************************************************/

void vanessa_socket_server_reaper(void);


/**********************************************************************
 * Logging functionality
 *
 * Depreciated, but provided for backwards compatibility.
 * Call vanessa_logger_set() and vanessa_logger_unset() instead
 *
 **********************************************************************/

/**********************************************************************
 * vanessa_socket_logger_set
 *
 * Depreciated, but provided for backwards compatibility.
 * Call vanessa_logger_set() instead
 *
 * set the logger function to use
 * No logging will take place if logger is set to NULL (default)
 * That is you _must_ call this function to enable logging.
 * pre: logger: pointer to a vanessa_logger
 * post: logger for vanessa_socket is set to logger
 * return: none
 **********************************************************************/

#define vanessa_socket_logger_set(_vl) vanessa_logger_set(_vl)


/**********************************************************************
 * vanessa_socket_logger_unset
 *
 * Depreciated, but provided for backwards compatibility.
 * Call vanessa_logger_unset() instead
 *
 * set logger to NULL
 * That is no logging will take place
 * pre: none
 * post: logger is NULL
 * return: none
 **********************************************************************/

#define vanessa_socket_logger_unset() vanessa_logger_unset()


/**********************************************************************
 * vanessa_socket_daemon_process
 * Close all file descriptors and fork to become a vanessa_socket_daemon.
 * Note: vanessa_socket_daemon_inetd_process should be called if the 
 * process is being run from inetd.
 **********************************************************************/

void vanessa_socket_daemon_process(void);


/**********************************************************************
 * vanessa_socket_daemon_inetd_process
 * Chdir to / and set umask to 0
 * This is all we really need to do if our process is run from
 * inetd
 **********************************************************************/

void vanessa_socket_daemon_inetd_process(void);


/**********************************************************************
 * vanessa_socket_daemon_become_child
 * Fork and exit from parent process. When we return 
 * we are our own clild. Very incestuous.
 **********************************************************************/

void vanessa_socket_daemon_become_child(void);


/**********************************************************************
 * vanessa_socket_daemon_close_fd
 * Close all the file descriptots a process has
 **********************************************************************/

void vanessa_socket_daemon_close_fd(void);


/**********************************************************************
 * vanessa_socket_daemon_setid
 * Set the userid and groupid of the process.
 * Arguments are the username or the userid as a string and 
 * the group or the groupid as a string.
 **********************************************************************/

int vanessa_socket_daemon_setid(const char *user, const char *group);


/**********************************************************************
 * vanessa_socket_daemon_exit_cleanly
 * If we get a sinal then close everthing, log it and quit
 **********************************************************************/

void vanessa_socket_daemon_exit_cleanly(int i);


/**********************************************************************
 * vanessa_socket_handler_noop
 * A signal handler that does nothing but reinstall itself
 * as the signal handler for the signal.
 * pre: sig: signal recieved by the process
 * post: signal handler reset for signal
 **********************************************************************/

void vanessa_socket_handler_noop(int sig);


/**********************************************************************
 * vanessa_socket_handler_reaper
 * A signal handler that waits for a signal and runs wait3 to free
 * the resources of any exited children. This stops zombie processes
 * from hanging around. Designed to listen for SIGCHLD
 * pre: sig: signal is recieved by the process
 * post: Resources of any exited children are freed
 *       Signal Handler for signal reset
 **********************************************************************/

void vanessa_socket_handler_reaper(int sig);

#endif
