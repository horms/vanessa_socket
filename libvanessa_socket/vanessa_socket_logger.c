/**********************************************************************
 * vanessa_socket_logger.c                                     May 2000
 * Horms                                             horms@vergenet.net
 *
 * Logging functionality
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
#include "vanessa_socket_logger.h"

/*
 * Its all about this gloabal
 */
vanessa_logger_t *vanessa_socket_logger = NULL;


/**********************************************************************
 * Note:
 * vanessa_socket_logger_set and vanessa_socket_logger_unset
 * are macros defined elsewhere
 **********************************************************************/
