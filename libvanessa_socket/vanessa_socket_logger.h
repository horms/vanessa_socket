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

#ifndef VANESSA_SOCKET_LOGGER_FLIM
#define VANESSA_SOCKET_LOGGER_FLIM


#include <errno.h>
#include "vanessa_socket.h"

extern vanessa_logger_t *vanessa_socket_logger;
extern int errno;

#define VANESSA_SOCKET_LOG(priority, fmt, args...) \
  vanessa_logger_log(vanessa_socket_logger, priority, fmt, ## args);

#define VANESSA_SOCKET_INFO(fmt, args...) \
  VANESSA_SOCKET_LOG(LOG_INFO, fmt, ## args);

#define VANESSA_SOCKET_ERR(fmt, args...) \
  VANESSA_SOCKET_LOG(LOG_ERR, fmt, ## args);

#define VANESSA_SOCKET_DEBUG(fmt, args...) \
  VANESSA_SOCKET_LOG(LOG_DEBUG, __FUNCTION__ ": " fmt, ## args);

#define VANESSA_SOCKET_DEBUG_ERRNO(s) \
  VANESSA_SOCKET_LOG(LOG_DEBUG, "%s: %s: %s", __FUNCTION__, s, strerror(errno));


#endif
