/**********************************************************************
 * vanessa_socket_server.c                               September 1999
 * Horms                                             horms@vergenet.net
 *
 * Open accpet a tcp connection from a client (we are the server)
 *
 * vanessa_socket
 * Library to simplify handling of TCP sockets
 * Copyright (C) 1999-2002  Horms
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

/*Keep track of the total number of connections in the parent process*/
unsigned int noconnection;


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
				     vanessa_socket_flag_t flag)
{
	int s;
	struct sockaddr_in from;

	/* Fill in informtaion for 'from' */
	if (vanessa_socket_host_port_sockaddr_in(interface_address,
						 port, &from, flag) < 0) {
		VANESSA_LOGGER_DEBUG("vanessa_socket_host_port_sockaddr_in");
		return (-1);
	}

	/* Make the connection */
	s = vanessa_socket_server_bind_sockaddr_in(from, flag); 
	if (s < 0) {
		VANESSA_LOGGER_DEBUG("vanessa_socket_server_bind");
		return (-1);
	}

	return (s);
}


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
					   vanessa_socket_flag_t flag)
{
	int s;
	int g;
	int addrlen;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("socket");
		return (-1);
	}

	/* 
	 * Set SO_REUSEADDR on the server socket s. Variable g is used
	 * as a scratch varable.
	 */
	g = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (void *) &g, sizeof g)
	    < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("setsockopt");
		close(s);
		return (-1);
	}
#ifdef SO_BINDANY
	g = 1;
	if (setsockopt(s, SOL_SOCKET, SO_BINDANY, (void *) &g, sizeof g) <
	    0) {
		VANESSA_LOGGER_DEBUG_ERRNO("setsockopt");
		close(s);
		return (-1);
	}
#endif

	addrlen = sizeof(struct sockaddr_in);

	if (bind(s, (struct sockaddr *) &from, addrlen) < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("bind");
		close(s);
		return (-1);
	}

	listen(s, 5);

	return(s);
}


/**********************************************************************
 * vanessa_socket_server_accept
 * Accept connections on a bound socket.
 * vanessa_socket_server_bind or vanessa_socket_server_bind_sockaddr_in
 * may be used to open the bound socket.
 * When one is received fork
 * In the Child: close the listening file descriptor
 *               return the file descriptor that is 
 *               a socket connection to the client
 * In the Server: close the socket to the client and loop
 * pre: port: port to listen to, an ASCII representation of a number
 *            or an entry from /etc/services
 *      interface_address: If NULL bind to all interfaces, else
 *                         bind to interface(es) with this address.
 *      maximum_connections: maximum number of active connections
 *                           to handle. If 0 then an number of connections 
 *                           is unlimited.
 *                           Not used if flat is VANESSA_SOCKET_NO_FORK
 *      return_from: pointer to a struct_in addr where the 
 *                   connecting client's IP address will
 *                   be placed. Ignored if NULL
 *      return_to: pointer to a in addr where the IP address the 
 *                 server accepted the connection on will be placed.
 *                 Ignored if NULL
 *      flag: If VANESSA_SOCKET_NO_FORK then the process does not for
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
				      struct sockaddr_in *return_from, 
				      struct sockaddr_in *return_to,
				      vanessa_socket_flag_t flag)
{
	int g;
	int addrlen;
	struct sockaddr_in from;

	extern unsigned int noconnection;

	addrlen = sizeof(from);

	for(;;) {
		g = accept(listen_socket, (struct sockaddr *) &from, &addrlen);
		if (g  < 0) {
			if(errno == EINTR || errno == ECONNABORTED) {
				continue; /* Ignore EINTR  and ECONNABORTED */
			}
			VANESSA_LOGGER_DEBUG_ERRNO("accept");
			return(-1);
		}

		if ( !(flag&VANESSA_SOCKET_NO_FORK) && maximum_connections && 
				noconnection >= maximum_connections) {
			close(g);
			g=-1;
			if(flag&VANESSA_SOCKET_NO_FORK) {
				break;
			}
		}

		if (return_to != NULL) {
			addrlen = sizeof(struct sockaddr_in);
			if (getsockname (g, (struct sockaddr *) return_to, 
					&addrlen) < 0) { 
				VANESSA_LOGGER_DEBUG_ERRNO ("getsockname"); 
				return (-1);
			}
		}

		if (return_from != NULL) {
			addrlen = sizeof(struct sockaddr_in);
			if (getpeername (g, (struct sockaddr *) return_from, 
					&addrlen) < 0) { 
				VANESSA_LOGGER_DEBUG_ERRNO ("getpeername"); 
				return (-1);
			}
		}

		if(flag&VANESSA_SOCKET_NO_FORK) {
			break;
		}

		if(fork() == 0) {
			if(close(listen_socket) < 0) {
				VANESSA_LOGGER_DEBUG_ERRNO("close");
				return(-1);
			}
			return(g);
		}
		else {
			noconnection++;
			close(g);
		}
	}

	return(g);
}


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
				  struct sockaddr_in *return_from,
				  struct sockaddr_in *return_to,
				  vanessa_socket_flag_t flag)
{
	struct sockaddr_in from;
	int s;

	/* Fill in informtaion for 'from' */
	if (vanessa_socket_host_port_sockaddr_in(interface_address,
						 port, &from, flag) < 0) {
		VANESSA_LOGGER_DEBUG
		    ("vanessa_socket_host_port_sockaddr_in");
		return (-1);
	}

	/* Make the connection */
	if ((s = vanessa_socket_server_connect_sockaddr_in(from,
							   maximum_connections,
							   return_from,
							   return_to,
							   flag)) < 0) {
		VANESSA_LOGGER_DEBUG
		    ("vanessa_socket_server_connect_sockaddr_in");
		return (-1);
	}

	return (s);
}


/**********************************************************************
 * vanessa_socket_server_connect_sockaddr_in
 * Listen on a tcp port for incoming client connections 
 * When one is received fork
 * In the Child: close the listening file descriptor
 *               return the file descriptor that is 
 *               a socket connection to the client
 * In the Server: close the socket to the client and loop
 * pre: from: sockaddr_in to bind to
 *      maximum_connections: maximum number of active connections to 
 *                           handle. If 0 then an number of connections 
 *                           is unlimited.
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
 *       if return_from is non-null, it is seeded with clients address
 * return: open client socket, if connection is accepted.
 *         -1 on error
 **********************************************************************/

int vanessa_socket_server_connect_sockaddr_in(struct sockaddr_in from,
					      const unsigned int
					      maximum_connections,
					      struct sockaddr_in
					      *return_from,
					      struct sockaddr_in
					      *return_to,
					      vanessa_socket_flag_t flag)
{
	int s;
	int g;

	s = vanessa_socket_server_bind_sockaddr_in(from, flag);
	if(s < 0) {
		VANESSA_LOGGER_DEBUG("vanessa_socket_server_bind_sockaddr_in");
		return (-1);
	}

	g = vanessa_socket_server_accept(s, maximum_connections, 
					 return_from, return_to, 0);
	if(g < 0) {
		VANESSA_LOGGER_DEBUG("vanessa_socket_server_accept");
		return(-1);
	}

	return(g);
}
