/**********************************************************************
 * vanessa_socket_host.c                                  December 1999
 * Horms                                             horms@verge.net.au
 *
 * Operations on host names
 *
 * vanessa_socket
 * Library to simplify handling of TCP sockets
 * Copyright (C) 1999-2008  Horms <horms@verge.net.au>
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
 * vanessa_socket_host_in_addr
 * A host is given as a string either as a host name or IP address
 * as a dotted quad. The host is used to seed an in_addr structure
 * with a binary representation of the IP address of the host
 * in network byte order.
 * pre: host: hostname or IP address
 *            If NULL then INADDR_ANY will be converted to network
 *            byte order and used as the address.
 *      in: pointer to an in_addr structure.
 *      flag: Flags. If the VANESSA_SOCKET_NO_LOOKUP bit is set then
 *            no hostname lookups will be performed. That is the
 *            host given as an argument should be an IP address
 * post: none
 * return: 0 on success
 *         -1 on error
 **********************************************************************/

int vanessa_socket_host_in_addr(const char *host,
				struct in_addr *in,
				const vanessa_socket_flag_t flag)
{
	struct hostent *hp;
	extern int h_errno;

	if (host == NULL) {
		in->s_addr = htonl(INADDR_ANY);
	} else if (flag & VANESSA_SOCKET_NO_LOOKUP) {
		if (inet_aton(host, in) == 0) {
			VANESSA_LOGGER_DEBUG_UNSAFE("invalid IP address (%s): "
					"Are you trying to resolve a hostname "
					"with no lookup enabled?", host);
			return (-1);
		}
	} else {
		if ((hp = gethostbyname(host)) == NULL) {
			VANESSA_LOGGER_DEBUG_UNSAFE("gethostbyname (%s): %s",
					host, 
					vanessa_logger_strherror(h_errno));
			return (-1);
		}
		memcpy(in, hp->h_addr, hp->h_length);
	}

	return (0);
}
