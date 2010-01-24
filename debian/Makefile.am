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

m4datadir = $(datadir)/aclocal
EXTRA_DIST = \
changelog \
compat \
control \
copyright \
libvanessa-socket-dev.dirs \
libvanessa-socket-dev.files \
libvanessa-socket-pipe.dirs \
libvanessa-socket-pipe.files \
libvanessa-socket2.dirs \
libvanessa-socket2.files \
libvanessa-socket2.postinst \
libvanessa-socket2.shlibs \
rules \
dpkg-checkbuild

clean-local:
	rm -f *.substvars *.debhelper files
	rm -r -f libvanessa-socket-dev libvanessa-socket0 tmp \
		libvanessa-socket-pipe

