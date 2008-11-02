/**********************************************************************
 * vanessa_socket_handler.c                                  March 2002
 * Simon Horman                                      horms@verge.net.au
 *
 * Signal handlers
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
 * vanessa_socket_handler_reaper
 * A signal handler that waits for a signal and runs wait3 to free
 * the resources of any exited children. This stops zombie processes
 * from hanging around. Designed to listen for SIGCHLD
 * pre: sig: signal is recieved by the process
 * post: Resources of any exited children are freed
 *       Signal handler for sig reset
 **********************************************************************/

void vanessa_socket_handler_reaper(int sig)
{
	int status;

	extern unsigned int noconnection;

	signal(sig, (void (*)(int)) vanessa_socket_handler_reaper);
	while (wait3(&status, WNOHANG, 0) > 0) {
		noconnection--;
	}
}


/**********************************************************************
 * vanessa_socket_handler_noop
 * A signal handler that does nothing but reinstall itself
 * as the signal handler for the signal.
 * pre: sig: signal recieved by the process
 * post: signal handler reset for signal
 **********************************************************************/

void vanessa_socket_handler_noop(int sig)
{
	signal(sig, (void (*)(int)) vanessa_socket_handler_noop);
}
