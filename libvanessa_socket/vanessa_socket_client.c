/**********************************************************************
 * vanessa_socket_client.c                                November 1999
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

#include "vanessa_socket.h"


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
){
  int out;

  /* Create socket for 'to' connection */
  if((out=socket(AF_INET, SOCK_STREAM, 0)) < 0){
    VANESSA_SOCKET_DEBUG_ERRNO("vanessa_socket_client_open: socket", errno);
    return(-1);
  }

  /* Connect to foreign 'to' server */
  if(connect(out, (struct sockaddr *)&to, sizeof(to))<0){
    VANESSA_SOCKET_DEBUG_ERRNO("vanessa_socket_client_open: connect", errno);
    return(-1);
  }

  return(out);
}



/**********************************************************************
 * vanessa_socket_client_open
 * Open a socket connection as a client
 * pre: host: hostname or ipaddress to open socket to
 *      port: name or number to open
 * post: socket is opened
 * flag: If VANESSA_SOCKET_NO_LOOKUP then no host and port lookups
 *       will be performed
 * return: open socket
 *         -1 on error
 **********************************************************************/

int vanessa_socket_client_open(
  const char *host, 
  const char *port, 
  const vanessa_socket_flag_t flag
){
  int out;
  struct sockaddr_in to;

  /* Fill in port information for 'to' */
  if(vanessa_socket_host_port_sockaddr_in(host, port, &to, flag)<0){
    VANESSA_SOCKET_DEBUG("vanessa_socket_client_open: "
      "vanessa_socket_host_port_sockaddr_in");
    return(-1);
  }

  /* Set up connection */
  if((out=vanessa_socket_client_open_sockaddr_in(to, flag)) < 0){
    VANESSA_SOCKET_DEBUG("vanessa_socket_client_open: "
		    "vanessa_socket_client_open_sockaddr_in");
    return(-1);
  }

  return(out);
}



