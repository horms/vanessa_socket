/**********************************************************************
 * vanessa_socket_host_port.c                                 July 2000
 * Horms                                             horms@verge.net.au
 *
 * Operations on host names and ports
 *
 * vanessa_socket
 * Library to simplify handling of TCP sockets
 * Copyright (C) 1999-2008  Horms
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
 * return: 0 on success
 *         -1 on error
 **********************************************************************/

int vanessa_socket_host_port_sockaddr_in(const char *host,
					 const char *port,
					 struct sockaddr_in *addr,
					 const vanessa_socket_flag_t flag)
{
	int portno;

	memset((struct sockaddr *) addr, 0, sizeof(*addr));

	/* Gratuitously assume the address will be an AF_INET address */
	addr->sin_family = AF_INET;

	if (vanessa_socket_host_in_addr(host, &(addr->sin_addr), flag) < 0) {
		VANESSA_LOGGER_DEBUG("vanessa_socket_host_in_addr");
		return (-1);
	}

	if ((portno = vanessa_socket_port_portno(port, flag)) < 0) {
		VANESSA_LOGGER_DEBUG("vanessa_socket_port_portno");
		return (-1);
	}
	addr->sin_port = (unsigned short int) portno;

	return (0);
}
