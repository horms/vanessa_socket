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


/*
 * Hooray for format string problems!
 *
 * Each of the logging macros has two versions. The UNSAFE version will
 * accept a format string. You should _NOT_ use the UNSAFE versions of the
 * first argument, the format string, is derived from user input. The safe
 * versions (versions that do not have the "_UNSAFE" suffix) do not accept
 * a format string and only accept one argument, the string to log. These
 * should be safe to use with user derived input.
 */

#define VANESSA_SOCKET_LOG_UNSAFE(priority, fmt, args...) \
  vanessa_logger_log(vanessa_socket_logger, priority, fmt, ## args);

#define VANESSA_SOCKET_LOG(priority, str) \
  vanessa_logger_log(vanessa_socket_logger, priority, "%s", str)

#define VANESSA_SOCKET_DEBUG_UNSAFE(fmt, args...) \
  vanessa_logger_log(vanessa_socket_logger, LOG_DEBUG, \
    __FUNCTION__ ": " fmt, ## args);

#define VANESSA_SOCKET_DEBUG(str) \
  vanessa_logger_log(vanessa_socket_logger, LOG_DEBUG, \
    __FUNCTION__ ": %s", str);

#define VANESSA_SOCKET_DEBUG_ERRNO(s) \
  vanessa_logger_log(vanessa_socket_logger, LOG_DEBUG, "%s: %s: %s", \
    __FUNCTION__, s, strerror(errno));

#endif
