######################################################################
# Makefile.am                                            February 2000
# Simon Horman                                      horms@verge.net.au
#
# vanessa_socket
# Library to simplify handling of TCP sockets
# Copyright (C) 1999-2008  Simon Horman <horms@verge.net.au>
# 
# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
# USA
#
######################################################################

lib_LTLIBRARIES = libvanessa_socket.la

include_HEADERS = vanessa_socket.h

libvanessa_socket_la_SOURCES = \
vanessa_socket.h \
vanessa_socket_client.c \
vanessa_socket_daemon.c \
vanessa_socket_handler.c \
vanessa_socket_pipe.c \
vanessa_socket_server.c \
unused.h

libvanessa_socket_la_LDFLAGS    = -version-info 3:0:1

libvanessa_socket_la_LIBADD = @extra_libs@ -lvanessa_logger
