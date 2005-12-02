######################################################################
# Makefile                                               December 2005
# Horms                                             horms@verge.net.au
#
# gethostbyname
# Expose gethostbyname as a shell command 
# Copyright (C) 1999-2005  Horms and others
#
# Based on code from libvanessa_socket and libvanessa_logger
# Copyright (C) 1999-2005  Horms and others
# http://www.vergenet.net/linux/vanessa/
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA
#
######################################################################/

bin_PROGRAMS = vanessa_gethostbyname

man_MANS = vanessa_gethostbyname.1

EXTRA_DIST = vanessa_socket_pipe_config.h.in $(man_MANS)

vanessa_gethostbyname_SOURCES = vanessa_gethostbyname.c

INCLUDES= -I$(top_srcdir)/libvanessa_socket

vanessa_gethostbyname_LDADD = \
-L../libvanessa_socket \
-L../libvanessa_socket/.libs/ \
-lvanessa_socket \
@extra_libs@ \
@vanessa_logger_libs@
