/**********************************************************************
 * vanessa_socket_client.c                                November 1999
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

#include "vanessa_socket.h"


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
					   flag)
{
	int s;
	struct sockaddr_in from;

	/* Connect to foreign 'to' server */
	s = vanessa_socket_client_open_src_sockaddr_in(from, to, 
			flag | VANESSA_SOCKET_NO_FROM);
	if (s < 0) {
		VANESSA_LOGGER_DEBUG
		    ("vanessa_socket_client_open_src_sockaddr_in");
		return (-1);
	}

	return (s);
}



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
			       const vanessa_socket_flag_t flag)
{
	int s;

	s = vanessa_socket_client_src_open(NULL, NULL, host, port, 
			flag | VANESSA_SOCKET_NO_FROM);
	if (s < 0) {
		VANESSA_LOGGER_DEBUG("vanessa_socket_client_src_open");
		return (-1);
	}

	return (s);
}


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
 *            If flag&VANESSA_SOCKET_TCP_KEEPALIVE then turn on
 *            TCP-Keepalive
 * post: socket is opened
 * return: open socket
 *         -1 on error
 **********************************************************************/

int vanessa_socket_client_open_src_sockaddr_in(struct sockaddr_in from,
					       struct sockaddr_in to,
					       const vanessa_socket_flag_t
					       flag)
{
	int out;
	int g;

	/* Create socket */
	memset((struct sockaddr *) &from, 0, sizeof(from));
	if ((out = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("socket");
		return (-1);
	}

	/* Turn on TCP-Keepalive */
	if (flag & VANESSA_SOCKET_TCP_KEEPALIVE) {
		g = 1;
		setsockopt(out, SOL_SOCKET, SO_KEEPALIVE, (void *) &g, sizeof g);
	}

	/* Bind 'from' to socket */
	if (!(flag & VANESSA_SOCKET_NO_FROM) ||
	    from.sin_addr.s_addr != INADDR_ANY ||
	    from.sin_port != INPORT_ANY) {
		if (bind
		    (out, (struct sockaddr *) &from,
		     sizeof(struct sockaddr_in)) < 0) {
			VANESSA_LOGGER_DEBUG_ERRNO("bind");
			return (-1);
		}
	}

	/* Connect to foreign 'to' server */
	if (connect(out, (struct sockaddr *) &to, sizeof(to)) < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("connect");
		return (-1);
	}

	return (out);
}



/**********************************************************************
 * vanessa_socket_client_src_open
 * Open a socket connection as a client
 * pre: src_host: hostname or ipaddress to open socket from
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
 *            If flag&VANESSA_SOCKET_TCP_KEEPALIVE then turn on
 *            TCP-Keepalive
 * post: socket is opened
 * return: open socket
 *         -1 on error
 **********************************************************************/

int vanessa_socket_client_src_open(const char *src_host,
				   const char *src_port,
				   const char *dst_host,
				   const char *dst_port,
				   const vanessa_socket_flag_t flag)
{
	int s, err;
	struct addrinfo hints, *dst_ai, *src_ai;
	struct addrinfo *dst_res = NULL, *src_res = NULL;
	int g;

	src_res = NULL;
	/* Get sockaddr list for source address */
	if ((src_host || src_port) && !(flag & VANESSA_SOCKET_NO_FROM)) {
		bzero(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		err = getaddrinfo(src_host, src_port, &hints, &src_res);
		if (err) {
			src_res = NULL;
			if (err == EAI_SYSTEM)
				VANESSA_LOGGER_DEBUG_UNSAFE("getaddrinfo src: "
							    "\"%s\" \"%s\": %s",
							    src_host, src_port,
							    strerror(errno));
			else
				VANESSA_LOGGER_DEBUG_UNSAFE("getaddrinfo src: "
							    "\"%s\" \"%s\": %s",
							    src_host, src_port,
							    gai_strerror(err));
			goto err;
		}
	}

	/* Get sockaddr list for destination address */
	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	err = getaddrinfo(dst_host, dst_port, &hints, &dst_res);
	if (err) {
		dst_res = NULL;
		if (err == EAI_SYSTEM)
			VANESSA_LOGGER_DEBUG_UNSAFE("getaddrinfo dst: "
						    "\"%s\" \"%s\": %s",
						    dst_host, dst_port,
						    strerror(errno));
		else
			VANESSA_LOGGER_DEBUG_UNSAFE("getaddrinfo dst: "
						    "\"%s\" \"%s\": %s",
						    dst_host, dst_port,
						    gai_strerror(err));
		goto err;
	}

	/* Try all combinations of destination and source until we get a
	 * connection.
	 */
	dst_ai = dst_res;
	do {
		/* Create socket */
		if ( (s = socket(dst_res->ai_family, dst_res->ai_socktype,
				 dst_res->ai_protocol)) < 0) {
			VANESSA_LOGGER_DEBUG_ERRNO("socket");
			continue;
		}

		/* Turn on TCP-Keepalive */
		if (flag & VANESSA_SOCKET_TCP_KEEPALIVE) {
			g = 1;
			setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (void *) &g,
				   sizeof g);
		}

		src_ai = src_res;
		/* Run through this loop at least once even if there is no 
		 * explicit source address.
		 */
		do {
			if (src_ai) {
				/* Bind source address to socket */
				if (bind(s, src_ai->ai_addr,
					 src_ai->ai_addrlen) < 0 ) {
					VANESSA_LOGGER_DEBUG_ERRNO("bind");
					continue;
				}
			}
			/* Connect to destination server */
			if (connect(s, dst_res->ai_addr,
				    dst_res->ai_addrlen) == 0)
				goto out;
			VANESSA_LOGGER_DEBUG_ERRNO("connect");
		} while (src_ai && (src_ai = src_ai->ai_next));

		if (close(s)) {
			VANESSA_LOGGER_DEBUG_ERRNO("close");
			goto err;
		}
	} while (dst_ai && (dst_ai = dst_ai->ai_next));

	VANESSA_LOGGER_DEBUG("vanessa_socket_client_src_open");
err:
	s = -1;
out:
	if (dst_res)
		freeaddrinfo(dst_res);
	if (src_res)
		freeaddrinfo(src_res);
	return s;
}


#ifdef THIS_CODE_IS_EXPERIMENTAL
/* Code below this line is Experimental */

int vanessa_socket_client_open_src_sockaddr_inv
    (struct sockaddr_in from, struct sockaddr_in *to, int tocount,
     const vanessa_socket_flag_t flag) {
	int s, i, hifd = 0, remaining = 0, ret = -1;
	fd_set connections;
	long opt = 1;
	struct timeval tv;

	FD_ZERO(&connections);
	tv.tv_sec = 120;
	tv.tv_usec = 0;

	for (i = 0; i < tocount; i++) {
		if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
			VANESSA_LOGGER_DEBUG_ERRNO("socket");
			goto out;
		}
		if (!(flag & VANESSA_SOCKET_NO_FROM)) {
			if (setsockopt
			    (s, SOL_SOCKET, SO_REUSEADDR, &opt,
			     sizeof(opt)))
				VANESSA_LOGGER_DEBUG_ERRNO
				    ("setsockopt(SO_REUSEADDR)");
			if (bind
			    (s, (struct sockaddr *) &from,
			     sizeof(from)) < 0) {
				VANESSA_LOGGER_DEBUG_ERRNO("bind");
				goto out;
			}
		}
		if (fcntl(s, F_SETFL, O_NDELAY))
			VANESSA_LOGGER_DEBUG_ERRNO("fcntl(O_NDELAY)");
		if (connect(s, (struct sockaddr *) (to + i), sizeof(*to)) <
		    0 && errno != EINPROGRESS) {
			VANESSA_LOGGER_DEBUG_ERRNO("connect");
			goto out;
		}
		FD_SET(s, &connections);
		if (s > hifd)
			hifd = s;
		remaining++;
	}

	while (tv.tv_sec > 0 || tv.tv_usec > 0) {
		struct timeval now, then, timeout;
		int j;
		fd_set rfds, wfds;

		memcpy(&timeout, &tv, sizeof(tv));
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		for (i = 0; i <= hifd; i++)
			if (FD_ISSET(i, &connections)) {
				FD_SET(i, &rfds);
				FD_SET(i, &wfds);
			}
		gettimeofday(&now, NULL);
		i = select(hifd + 1, &rfds, &wfds, NULL, &timeout);
		if (i < 0) {
			VANESSA_LOGGER_DEBUG_ERRNO("select");
			goto out;
		}
		if (!i)
			goto out;
		gettimeofday(&then, NULL);
		timeradd(&tv, &now, &tv);
		timersub(&tv, &then, &tv);
		for (j = 0; j <= hifd; j++) {
			int serr, serrlen;

			serrlen = sizeof(serr);
			if (FD_ISSET(j, &wfds) || FD_ISSET(j, &rfds)) {
				if (!getsockopt
				    (j, SOL_SOCKET, SO_ERROR, &serr,
				     &serrlen) && !serr) {
					ret = j;
					goto out;
				}
				close(j);
				FD_CLR(j, &connections);
			}
		}
	}

      out:
	for (i = 0; i <= hifd; i++) {
		if (i == ret)
			continue;
		if (FD_ISSET(i, &connections))
			close(i);
	}
	if (ret >= 0)
		if (fcntl(ret, F_SETFL, O_NDELAY))
			VANESSA_LOGGER_DEBUG_ERRNO("fcntl(O_NDELAY)");
	return ret;
}


int vanessa_socket_host_port_sockaddr_inv(const char *host,
					  const char *port,
					  struct sockaddr_in **addr,
					  int *addrcount,
					  const vanessa_socket_flag_t flag)
{
	int portno;
	int i, count = 0;
	struct hostent *hp;

	memset((struct sockaddr *) addr, 0, sizeof(addr));

	if ((hp = gethostbyname(host)) == NULL) {
		VANESSA_LOGGER_DEBUG_HERRNO("gethostbyname");
		return (-1);
	}
	while (hp->h_addr_list[++count]);
	*addr = malloc(sizeof(**addr) * count);

	if ((portno = vanessa_socket_port_portno(port, flag)) < 0) {
		VANESSA_LOGGER_DEBUG("vanessa_socket_port_portno");
		return (-1);
	}
	for (i = 0; i < count; i++) {
		(*addr)[i].sin_family = AF_INET;
		(*addr)[i].sin_port = (unsigned short int) portno;
		memcpy(&(*addr)[i].sin_addr.s_addr, hp->h_addr_list[i],
		       hp->h_length);
	}
	*addrcount = count;
	return 0;
}

#endif				/* THIS_CODE_IS_EXPERIMENTAL */
