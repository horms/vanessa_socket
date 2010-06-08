######################################################################
# Makefile.am                                           September 2000
# Simon Horman                                      horms@verge.net.au
#
# vanessa_socket_pipe
# Trivial TCP/IP pipe based on libvanessa_socket
# Copyright (C) 1999-2008  Simon Horman <horms@verge.net.au>
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307  USA
#
######################################################################

bin_PROGRAMS = vanessa_socket_pipe

man_MANS = vanessa_socket_pipe.1

EXTRA_DIST = vanessa_socket_pipe_config.h.in COPYING vanessa_socket_pipe.1

vanessa_socket_pipe_SOURCES = \
  vanessa_socket_pipe.c \
  vanessa_socket_pipe_config.h \
  options.h \
  options.c

INCLUDES= -I$(top_srcdir)/libvanessa_socket

vanessa_socket_pipe_LDADD = \
-L../libvanessa_socket \
-L../libvanessa_socket/.libs/ \
-lvanessa_socket \
-lvanessa_logger \
@extra_libs@ \
@vanessa_logger_libs@ \
-lpopt
