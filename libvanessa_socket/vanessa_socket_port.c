/**********************************************************************
 * vanessa_socket_port.c                                  December 1999
 * Simon Horman                                      horms@verge.net.au
 *
 * Operations on port numbers
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
 * vanessa_socket_port_portno
 * port number of a service given as a string either
 * the port number or the service name as per /etc/services
 * pre: port name as per /etc/services or port number as a string
 *      flag: Flags. If the VANESSA_SOCKET_NO_LOOKUP bit is set then
 *            no service lookups will be performed. That is the
 *            port given as an argument should be an port number
 *            If (flag & VANESSA_SOCKET_PROTO_MASK) == VANESSA_SOCKET_PROTO_UDP
 *            then lookup a udp service. Else lookup a tcp service.
 * return: port number
 *         -1 on error or if port name cannot be found in /etc/services
 **********************************************************************/

long int vanessa_socket_port_portno(const char *port,
			       const vanessa_socket_flag_t flag)
{
	const char *proto_str;
	struct servent *ent;
	long int portno;

	if((flag & VANESSA_SOCKET_PROTO_MASK) == VANESSA_SOCKET_PROTO_UDP) {
		proto_str = VANESSA_SOCKET_PROTO_STR_UDP;
	}
	else {
		proto_str = VANESSA_SOCKET_PROTO_STR_TCP;
	}

	if (port == NULL) {
		portno = INPORT_ANY;
	} else if (vanessa_socket_str_is_digit(port)) {
		portno = htons(atol(port));
	} else if (flag & VANESSA_SOCKET_NO_LOOKUP) {
		/* Must be a port name.
		 * But we're not doing lookups.
		 * Uh oh! */
		VANESSA_LOGGER_DEBUG("port is non-numeric "
				"and no lookups has been requested");
		return(-1);
	} else {
		if ((ent = getservbyname(port, proto_str)) == NULL) {
			VANESSA_LOGGER_DEBUG("could not find service");
			return (-1);
		}
		portno = (long int)ent->s_port;
	}

	if(portno < 0 || portno >= 65535) {
		VANESSA_LOGGER_DEBUG("port out of range");
		return(-1);
	}

	return (portno);
}


/**********************************************************************
 * vanessa_socket_str_is_digit
 * Test if a null terminated string is composed entirely of digits (0-9)
 * pre: String
 * return: 1 if string contains only digits and null terminator
 *         0 if string is NULL
 *         0 otherwise
 **********************************************************************/

int vanessa_socket_str_is_digit(const char *str)
{
	int offset;

	if (str == NULL) {
		return (0);
	}

	for (offset = strlen(str) - 1; offset > -1; offset--) {
		if (!isdigit((int) *(str + offset))) {
			break;
		}
	}

	return (offset > -1 ? 0 : 1);
}
