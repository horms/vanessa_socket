/**********************************************************************
 * vanessa_socket.h                                       November 1999
 * Horms                                             horms@vergenet.net
 *
 * Open a tcp socket to a server (we are the client)
 *
 * vanessa_socket
 * Library to simplify handling of TCP sockets
 * Copyright (C) 2000  Horms
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
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <syslog.h>

#include <vanessa_logger.h>

typedef unsigned int vanessa_socket_flag_t;

#define VANESSA_SOCKET_NONE      (vanessa_socket_flag_t) 0
#define VANESSA_SOCKET_NO_LOOKUP (vanessa_socket_flag_t) 1


/**********************************************************************
 * vanessa_socket_client_open_sockaddr_in
 * Open a socket connection as a client
 * pre: sockaddr_in: sockaddr structure specifying host and port to
 *      connect to.
 *      flag: ignored
 * post: socket is opened
 * flag: If VANESSA_SOCKET_NO_LOOKUP then no host and port lookups
 *       will be performed
 * return: open socket
 *         -1 on error
 **********************************************************************/

int vanessa_socket_client_open_sockaddr_in(
  struct sockaddr_in to,
  const vanessa_socket_flag_t flag
);


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

int vanessa_socket_client_open(
  const char *host, 
  const char *port, 
  const vanessa_socket_flag_t flag
);


/**********************************************************************
 * vanessa_socket_port_portno
 * port number of a service given as a string either
 * the port number or the service name as per /etc/services
 * pre: port name as per /etc/services or port number as a string
 *      flag: Flags. If the VANESSA_SOCKET_NO_LOOKUP bit is set then
 *            no service lookups will be performed. That is the
 *            port given as an argument should be an port number
 * return: port number
 *         0 on error or if port name cannot be found in /etc/services
 **********************************************************************/

unsigned short int vanessa_socket_port_portno(
  const char *port, 
  const vanessa_socket_flag_t flag
);


/**********************************************************************
 * vanessa_socket_str_is_digit
 * Test if a null terminated string is composed entirely of digits (0-9)
 * pre: String
 * return: 1 if string contains only digits and null teminator
 *         0 if string is NULL
 *         0 otherwise
 **********************************************************************/


int vanessa_socket_str_is_digit(const char *str);


/**********************************************************************
 * vanessa_socket_pipe
 * pipe data between two file discriptors until there is an error,
 * timeout or one or both the file descriptors are closed.
 * pre: fd_a_in:  one of the file descriptors to read from
 *      fd_a_out: one of the file descriptors to write to
 *      fd_b_in:  one of the file descriptors to read from
 *      fd_b_out: one of the file descriptors to write to
 *      buffer:   allocated buffer to read data into
 *      buffer_length: size of buffer in bytes
 *      idle_timeout:  timeout in seconds to wait for input
 *                     timout of 0 = infinite timeout
 *      return_a_read_bytes: Pointer to int where number
 *                           of bytes read from a will be recorded.
 *      return_b_read_bytes: Pointer to int where number
 *                           of bytes read from b will be recorded.
 * post: data is read from fd_a_in and written to fd_b_out and
 *       data is read from fd_b_in and written to fd_a_out.
 * return: -1 on error
 *         1 on idle timeout
 *         0 otherwise (one of fd_a_in or fd_b_in closes gracefully)
 **********************************************************************/

int vanessa_socket_pipe(
  int fd_a_in, 
  int fd_a_out, 
  int fd_b_in, 
  int fd_b_out, 
  unsigned char *buffer, 
  int buffer_length,
  int timeout,
  int *return_a_read_bytes,
  int *return_b_read_bytes
);


/**********************************************************************
 * vanessa_socket_pipe_read_write
 * Read data from one file descriptor (socket) and write it to another
 * pre: in_fd: file descriptor to read from
 *      out_fd: file descriptor to write to
 *      buffer: allocated buffer to store read data in
 *      buffer_length: size of the buffer
 * post: at most buffer_length bytes are read from in_fd and written 
 *       to out_fd. 
 * return: bytes read on sucess
 *         0 on EOF
 *         -1 on error
 **********************************************************************/

int vanessa_socket_pipe_read_write(
  int in_fd, 
  int out_fd, 
  unsigned char *buffer,
  int buffer_length
);


/**********************************************************************
 * vanessa_socket_pipe_write_bytes
 * write a n bytes of a buffer to fd
 * Pre: fd: File descriptor to write to
 *      buffer: buffer to write
 *      n: number or bytes to write
 * Return: -1 on error
 *         0 otherwise
 **********************************************************************/

int vanessa_socket_pipe_write_bytes(
  const int fd, 
  const unsigned char *buffer, 
  const ssize_t n
);


/**********************************************************************
 * vanessa_socket_server_connect
 * Listen on a tcp port for incoming client connections 
 * When one is recieved fork
 * In the Child: close the listening file desctiptor
 *               return the file descriptor that is 
 *               a socket connection to the client
 * In the Server: close the socket to the client and loop
 * pre: port: port to listen to, an ascii representation of a number
 *            or an entry from /etc/services
 *      interface_address: If NULL bind to all interfaces, else
 *                         bind to interface(es) with this address.
 *      maximum_connections: maximum number of active connections
 *      to handle. If 0 then an number of connections is unlimited.
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
 *       if return_from is non-null, it is seeded with cleints address
 * return: open client socket, if connection is accepted.
 *         -1 on error
 **********************************************************************/

int vanessa_socket_server_connect(
  const char *port,
  const char *interface_address,
  const unsigned int maximum_connections,
  struct sockaddr_in *return_from,
  struct sockaddr_in *return_to,
  vanessa_socket_flag_t flag
);


/**********************************************************************
 * vanessa_socket_server_connect_sockaddr_in
 * Listen on a tcp port for incoming client connections 
 * When one is recieved fork
 * In the Child: close the listening file desctiptor
 *               return the file descriptor that is 
 *               a socket connection to the client
 * In the Server: close the socket to the client and loop
 * pre: port: port to listen to, an ascii representation of a number
 *            or an entry from /etc/services
 *      interface_address: If NULL bind to all interfaces, else
 *                         bind to interface(es) with this address.
 *      maximum_connections: maximum number of active connections
 *      to handle. If 0 then an number of connections is unlimited.
 *      return_from: pointer to a struct_in addr where the 
 *                   connecting client's IP address will
 *                   be placed. Ignored if NULL
 *      return_to: pointer to a in addr where the IP address the 
 *                 server accepted the connection on will be placed.
 *                 Ignored if NULL
 *      flag: ignored
 * post: Client sockets are returned in child processes
 *       In the parent process the function doesn't exit, other 
 *       than on error.
 *       if return_from is non-null, it is seeded with cleints address
 * return: open client socket, if connection is accepted.
 *         -1 on error
 **********************************************************************/

int vanessa_socket_server_connect_sockaddr_in(
  struct sockaddr_in from,
  const unsigned int maximum_connections,
  struct sockaddr_in *return_from,
  struct sockaddr_in *return_to,
  vanessa_socket_flag_t flag
);


/**********************************************************************
 * vanessa_socket_server_reaper
 * A signal handler that waits for SIGCHLD and runs wait3 to free
 * the resources of any exited children. This stops zombie processes
 * from hanging around.
 * pre: SIGCHLD is recieved by the process
 * post: Resoerces of any exited children are freed
 * return: 0
 **********************************************************************/

void vanessa_socket_server_reaper(void);


/**********************************************************************
 * vanessa_socket_host_in_addr
 * A host is given as a string either as a host name or IP address
 * as a dotted quad. The host is used to seed an in_addr structure
 * with a binary representation fo the IP address of the host
 * in network byte order.
 * pre: host: hostname or IP address
 *            If NULL then INADDR_ANY will be converted to network
 *            byte order and used as the address.
 *      in: pointer to an in_addr structure.
 *      flag: Flags. If the VANESSA_SOCKET_NO_LOOKUP bit is set then
 *            no hostname lookups will be performed. That is the
 *            host given as an argument should be an IP address
 * post: none
 * return: 0 on sucess
 *         -1 on error
 **********************************************************************/

int vanessa_socket_host_in_addr(
  const char *host, 
  struct in_addr *in,
  const vanessa_socket_flag_t flag
);


/**********************************************************************
 * vanessa_socket_host_port_sockaddr_in
 * A host is given as a string either as a host name or IP address
 * as a dotted quad. A port number of a service is given as either a the
 * port number of the service name as per /etc/services. This is used
 * to seed a sockaddr_in structure.
 * pre: host: hostname or IP address
 *            If NULL then INADDR_ANY will be converted to network
 *            byte order and used as the address.
 *      port: port name as per /etc/services or port number as a string
 *      addr: pointer to an sockaddr_in structure.
 *      flag: Flags. If the VANESSA_SOCKET_NO_LOOKUP bit is set then
 *            no lookups will be performed. That is the
 *            host given as an argument should be an IP address and
 *            the port should be a port number
 * post: none
 * return: 0 on sucess
 *         -1 on error
 **********************************************************************/

int vanessa_socket_host_port_sockaddr_in(
  const char *host,
  const char *port,
  struct sockaddr_in *addr,
  const vanessa_socket_flag_t flag
);


/**********************************************************************
 * Logging functionality
 **********************************************************************/

extern vanessa_logger_t *vanessa_socket_logger;

#define VANESSA_SOCKET_DEBUG_ERRNO(s, e) \
  vanessa_logger_log(vanessa_socket_logger,LOG_DEBUG,"%s: %s",s,strerror(e));

#define VANESSA_SOCKET_DEBUG(s) \
  vanessa_logger_log(vanessa_socket_logger, LOG_DEBUG, s);


/**********************************************************************
 * vanessa_socket_logger_set
 * set the logger funtion to use
 * No logging will take place if logger is set to NULL (default)
 * That is you _must_ call this function to enable logging.
 * pre: logger: pointer to a vanessa_logger
 * post: logger for vanessa_socket is set to logger
 * return: none
 **********************************************************************/

#define vanessa_socket_logger_set(_vl) vanessa_socket_logger=_vl


/**********************************************************************
 * vanessa_socket_logger_unset
 * set logger to NULL
 * That is no logging will take place
 * pre: none
 * post: logger is NULL
 * return: none
 **********************************************************************/

#define vanessa_socket_logger_unset() vanessa_socket_logger_set(NULL)


#endif
