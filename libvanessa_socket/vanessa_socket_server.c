/**********************************************************************
 * vanessa_socket_server.c                               September 1999
 * Simon Horman                                      horms@verge.net.au
 *
 * Open accpet a tcp connection from a client (we are the server)
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

#include <sys/poll.h>

#include "vanessa_socket.h"
#include "unused.h"

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
 *            If VANESSA_SOCKET_TCP_KEEPALIVE then turn on
 *            TCP-Keepalive
 * post: Bound socket is returned
 * return: socket
 *         -1 on error
 **********************************************************************/

int vanessa_socket_server_bind(const char *port,
				     const char *interface_address,
				     vanessa_socket_flag_t flag)
{
	int s, g, err;
	struct addrinfo hints, *res;

	/* Get addrinfo list for the listening address */
	bzero( &hints, sizeof hints );
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	err = getaddrinfo(interface_address, port, &hints, &res);
	if (err) {
		if (err == EAI_SYSTEM)
			VANESSA_LOGGER_DEBUG_ERRNO("getaddrinfo");
		else
			VANESSA_LOGGER_DEBUG_UNSAFE("getaddrinfo: %s",
						    gai_strerror(err));
		return -1;
	}

	/* Loop through all returned addrinfo until we successfully listen */
	do {
		if ((s = socket(res->ai_family, res->ai_socktype,
				res->ai_protocol)) < 0) {
			VANESSA_LOGGER_DEBUG_ERRNO("socket");
			continue;
		}
	        /* Set SO_REUSEADDR on the server socket s.
		 * Variable g is used as a scratch varable.
	         */
		g = 1;
		if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (void *)&g,
			       sizeof(g)) < 0) {
			VANESSA_LOGGER_DEBUG_ERRNO("setsockopt");
			if (close(s))
				goto err_close;
			continue;
		}
	        /* Set SO_KEEPALIVE on the server socket s.
		 * Variable g is used as a scratch varable.
	         */
		if (flag & VANESSA_SOCKET_TCP_KEEPALIVE) {
			g = 1;
			setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (void *) &g,
				   sizeof g);
		}
#ifdef SO_BINDANY
		g = 1;
		if (setsockopt(s, SOL_SOCKET, SO_BINDANY, (void *)&g,
			       sizeof(g)) < 0) {
			VANESSA_LOGGER_DEBUG_ERRNO("setsockopt");
			if (close(s))
				goto err_close;
			continue;
		}
#endif
		if (bind(s, res->ai_addr, res->ai_addrlen) < 0) {
			VANESSA_LOGGER_DEBUG_ERRNO("bind");
			if (close(s))
				goto err_close;
			continue;
		}
		if ((listen(s, SOMAXCONN))) {
			VANESSA_LOGGER_DEBUG_ERRNO("listen");
			if (close(s))
				goto err_close;
			continue;
		}
		return s;
	} while ((res = res->ai_next));

	VANESSA_LOGGER_DEBUG("could not bind to any of the supplied addresses");
	freeaddrinfo(res);
	return -1;

err_close:
	VANESSA_LOGGER_DEBUG_ERRNO("close");
	freeaddrinfo(res);
	return -1;
}


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
			    vanessa_socket_flag_t flag)
{
	int *s;
	size_t ns;

	for (ns = 0; fromv[ns]; ns++)
		;

	s = (int *) malloc(sizeof(int) * (ns + 1));
	if (!s) {
		VANESSA_LOGGER_DEBUG_ERRNO("malloc");
		return NULL;
	}

	for (ns = 0; fromv[ns * 2]; ns++) {
		s[ns] = vanessa_socket_server_bind(fromv[(ns * 2) + 1], 
						   fromv[(ns * 2)], flag);
		if (s[ns] >= 0)
			continue;
		VANESSA_LOGGER_DEBUG("vanessa_socket_server_bind_sockaddr_in");
		if (vanessa_socket_closev(s) < 0) {
			VANESSA_LOGGER_DEBUG("vanessa_socket_closev");
		}
		return NULL;
	}
	*(s + ns) = -1;

	return (s);
}


/**********************************************************************
 * vanessa_socket_server_bind_sockaddr_in
 * Open a socket and bind it to a port and address
 * pre: from: sockaddr_in to bind to
 *      flag: If VANESSA_SOCKET_TCP_KEEPALIVE then turn on
 *            TCP-Keepalive
 * post: Bound socket
 * return: socket
 *         -1 on error
 **********************************************************************/

int vanessa_socket_server_bind_sockaddr_in(struct sockaddr_in from,
					   vanessa_socket_flag_t flag)
{
	int s;
	int g;
	unsigned int addrlen;

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
		if (close(s) < 0)
			VANESSA_LOGGER_DEBUG_ERRNO("warning: close");
		return (-1);
	}
	/* 
	 * Set SO_KEEPALIVE on the server socket s. Variable g is used
	 * as a scratch varable.
	 */
	if (flag & VANESSA_SOCKET_TCP_KEEPALIVE) {
		g = 1;
		setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (void *) &g, sizeof g);
	}
#ifdef SO_BINDANY
	g = 1;
	if (setsockopt(s, SOL_SOCKET, SO_BINDANY, (void *) &g, sizeof g) <
	    0) {
		VANESSA_LOGGER_DEBUG_ERRNO("setsockopt");
		if (close(s) < 0) 
			VANESSA_LOGGER_DEBUG_ERRNO("warning: close");
		return (-1);
	}
#endif

	addrlen = sizeof(struct sockaddr_in);

	if (bind(s, (struct sockaddr *) &from, addrlen) < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("bind");
		if (close(s) < 0)
			VANESSA_LOGGER_DEBUG_ERRNO("warning: close");
		return (-1);
	}

#ifdef SOMAXCONN
	listen(s, SOMAXCONN);
#else
	listen(s, 5);
#endif

	return(s);
}


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
					vanessa_socket_flag_t flag)
{
	int *s;
	size_t ns;

	s = (int *) malloc(sizeof(int) * (nfrom + 1));
	if (!nfrom) {
		VANESSA_LOGGER_DEBUG_ERRNO("malloc");
		return NULL;
	}

	for (ns = 0; ns < nfrom; ns++) {
		s[ns] = vanessa_socket_server_bind_sockaddr_in(fromv[ns], flag);
		if (s[ns] >= 0)
			continue;
		VANESSA_LOGGER_DEBUG("vanessa_socket_server_bind_sockaddr_in");
		if (vanessa_socket_closev(s) < 0)
			VANESSA_LOGGER_DEBUG("vanessa_socket_closev");
		return NULL;
	}
	*(s + ns) = -1;

	return s;
}


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
vanessa_socket_closev(int *sockv)
{
	int status = 0;
	int tmp_status;
	int *p;

	for (p = sockv; *p >= 0; p++) {
		tmp_status = close(*p);
		if (tmp_status < 0) {
			VANESSA_LOGGER_DEBUG_ERRNO("warning: close");
			if (!status)
				status = tmp_status;
		}
	}
	
	free (sockv);

	return status;
}


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




static pid_t 
__vanessa_socket_server_accept(int *g, int listen_socket, int *listen_socketv,
				      const unsigned int maximum_connections,
				      struct sockaddr *return_from, 
				      struct sockaddr *return_to,
				      vanessa_socket_flag_t flag)
{
	unsigned int addrlen;
	pid_t child = 0;
	struct sockaddr_storage from;

	extern unsigned int noconnection;

	*g = -1;

	for(;;) {
		addrlen = sizeof(from);
		*g = accept(listen_socket, (struct sockaddr *) &from, &addrlen);
		if (*g  < 0) {
			if(errno == EINTR || errno == ECONNABORTED) {
				continue; /* Ignore EINTR  and ECONNABORTED */
			}
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return -1; /* Don't log EAGAIN or EWOULDBLOCK */
			VANESSA_LOGGER_DEBUG_ERRNO("accept");
			return(-1);
		}
	
		if (flag & VANESSA_SOCKET_NO_FORK)
			break;

		if(maximum_connections && 
		   noconnection >= maximum_connections) {
			VANESSA_LOGGER_DEBUG("too many connections");
			goto err;
		}
		
		child = fork();
		if (child < 0) {
			VANESSA_LOGGER_DEBUG_ERRNO("fork");
			goto err;
		}
		else if(!child)
			break;

		/* Parent */
		noconnection++;
		if(close(*g) < 0) {
			VANESSA_LOGGER_DEBUG_ERRNO("warning: close");
			return -1;
		}

		return child;
	}

	/* Child */
	if (listen_socketv) {
		if(vanessa_socket_closev(listen_socketv) < 0) {
			VANESSA_LOGGER_DEBUG("vanessa_socket_closev");
			goto err;
		}
	}
	else {
		if (close(listen_socket) < 0) {
			VANESSA_LOGGER_DEBUG_ERRNO("warning: close");
			goto err;
		}
	}

	/* 'from', 'return_to', and 'return_from' are in the same address
	   family so the sockaddr lengths are identical. */
	if (return_to) {
		if (getsockname (*g, (struct sockaddr *) return_to, 
				&addrlen) < 0) { 
			VANESSA_LOGGER_DEBUG_ERRNO("getsockname"); 
			return -1;
		}
	}

	if (return_from)
		memcpy(return_from, &from, addrlen);

	return 0;
err:
	if (*g >= 0)
		if (close(*g) < 0)
			VANESSA_LOGGER_DEBUG_ERRNO("warning: close");
	return -1;
}


int vanessa_socket_server_accept(int listen_socket,
				      const unsigned int maximum_connections,
				      struct sockaddr *return_from, 
				      struct sockaddr *return_to,
				      vanessa_socket_flag_t flag)
{
	pid_t child;
	int g;

	while (1) {
		child = __vanessa_socket_server_accept(&g, listen_socket, NULL,
		 				       maximum_connections, 
						       return_from, return_to, 
						       flag);
		if (child < 0) {
			VANESSA_LOGGER_DEBUG("__vanessa_socket_server_accept");
			return -1;
		}

		if (flag & VANESSA_SOCKET_NO_FORK || child)
			return g;
	}

	/* Not Reached */
	return -1;
}


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

static int
__vanessa_socket_server_acceptv(int *g, int listen_socket, int *listen_socketv,
			       const unsigned int maximum_connections,
			       struct sockaddr *return_from, 
			       struct sockaddr *return_to,
			       vanessa_socket_flag_t flag)
{
	long opt;
	pid_t child;
	int status = 0;
		
	/* NONBLOCK must be set else accept() might block */
	opt = fcntl(listen_socket, F_GETFL, NULL);
	if (opt < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("fcntl: F_GETFL");
		return -1;
	}
	if (!(opt & O_NONBLOCK) && 
	    fcntl(listen_socket, F_SETFL, opt | O_NONBLOCK) < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("fcntl: F_SETFL 1");
		return -1;
	}

	status = child = __vanessa_socket_server_accept(g, listen_socket,
							listen_socketv,
							maximum_connections,
							return_from, return_to,
							flag);
	if (child < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			status = 0;
		else {
			VANESSA_LOGGER_DEBUG("__vanessa_socket_server_accept");
			status = -1;
		}
	}

	if (!(opt & O_NONBLOCK) && child &&
	    fcntl(listen_socket, F_SETFL, opt) < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("fcntl: F_SETFL 2");
		status = -1;
	}
 
	if (child < 0)
		return status;

	if (!(opt & O_NONBLOCK) && (flag & VANESSA_SOCKET_NO_FORK || !child) &&
	    fcntl(*g, F_SETFL, opt) < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("fcntl: F_SETFL 3");
		status = -1;
	}
	
	return status;
}


int 
vanessa_socket_server_acceptv(int *listen_socketv,
				      const unsigned int maximum_connections,
				      struct sockaddr *return_from, 
				      struct sockaddr *return_to,
				      vanessa_socket_flag_t flag)
{
	int g;
	int status = -1;
	size_t nfds;
	size_t i;
	struct pollfd *ufds;

	for (nfds = 0; listen_socketv[nfds] >= 0; nfds++)
		;

	ufds = (struct pollfd *)malloc(sizeof(struct pollfd) * nfds);
	if (!ufds) {
		VANESSA_LOGGER_DEBUG_ERRNO("malloc");
		return -1;
	}

	for (i = 0; i < nfds; i++) {
		ufds[i].fd = listen_socketv[i];
		ufds[i].events = POLLIN;
	}

	for (;;) {
		size_t i;

		status = poll(ufds, nfds, -1);
		if (status < 0) {
			if (errno == EINTR)
				continue;
			VANESSA_LOGGER_DEBUG_ERRNO("poll");
			goto out;
		}

		for (i = 0; i < nfds && status; i++) {
			pid_t child;
		
			if (!ufds[i].revents)
				continue;
			status--;

			/* POLLIN should be set if we get this far */
			child = __vanessa_socket_server_acceptv(&g, ufds[i].fd,
						listen_socketv, 
						maximum_connections, 
						return_from, return_to, flag);
			if (child < 0) {
				VANESSA_LOGGER_DEBUG(
					"__vanessa_socket_server_acceptv");
				goto err;
			}
			if (flag & VANESSA_SOCKET_NO_FORK || !child) {
				status = g;
				goto out;
			}
		}
	}

err:
	status = -1;
out:
	free(ufds);
	return status;
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
				  struct sockaddr *return_from,
				  struct sockaddr *return_to,
				  vanessa_socket_flag_t flag)
{
	const char *fromv[3];

	if (interface_address)
		fromv[0] = interface_address;
	else
		fromv[0] = "0.0.0.0";
	fromv[1] = port;
	fromv[2] = NULL;

	return vanessa_socket_server_connectv(fromv, maximum_connections,
					      return_from, return_to,
					      flag);
}


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
vanessa_socket_server_connectv(const const char **fromv,
			       const unsigned int maximum_connections,
			       struct sockaddr *return_from,
			       struct sockaddr *return_to,
			       vanessa_socket_flag_t flag)
{
	int *s;
	int g;

	s = vanessa_socket_server_bindv(fromv, flag);
	if(*s < 0) {
		VANESSA_LOGGER_DEBUG("vanessa_socket_server_bind_sockaddr_in");
		return (-1);
	}

	g = vanessa_socket_server_acceptv(s, maximum_connections, 
					 return_from, return_to, 0);
	if(g < 0) {
		if (vanessa_socket_closev(s) < 0)
			VANESSA_LOGGER_DEBUG("vanessa_socket_closev");
		VANESSA_LOGGER_DEBUG("vanessa_socket_server_accept");
		return(-1);
	}

	return (g);
}
