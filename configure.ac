dnl vanessa_socket
dnl Library to simplify handling of TCP sockets
dnl Copyright (C) 1999-2008  Simon Horman <horms@verge.net.au>
dnl 
dnl This library is free software; you can redistribute it and/or modify it
dnl under the terms of the GNU Lesser General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or (at
dnl your option) any later version.
dnl 
dnl This library is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
dnl General Public License for more details.
dnl 
dnl You should have received a copy of the GNU Lesser General Public License
dnl along with this library; if not, write to the Free Software Foundation,
dnl Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

AC_INIT(libvanessa_socket/vanessa_socket.h)
AM_CONFIG_HEADER(config.h)

AC_CONFIG_MACRO_DIR([libltdl/m4])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
LT_INIT
LTDL_INIT
AM_INIT_AUTOMAKE(vanessa_socket, 0.0.12)

AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_CHECK_LIB(socket, socket, [ socket_lib="-lsocket" ], :)
AC_CHECK_LIB(nsl, gethostbyname, [ nsl_lib="-lnsl" ], :)
AC_CHECK_LIB(resolv, inet_aton, [ resolv_lib="-lresolv" ], :)

AC_CHECK_MEMBERS([struct sockaddr.sa_len], [], [], [[#include <sys/socket.h>]])

AC_MSG_CHECKING("for Linux style shared object linking")
if test "$host_os" = "linux" -o "$host_os" = "linux-gnu"; then
  AC_MSG_RESULT(Yes)
  vanessa_logger_libs=""
  extra_libs=""
else
  AC_MSG_RESULT(No)
  vanessa_logger_libs="-lvanessa_logger"
  extra_libs="$socket_lib $nsl_lib $resolv_lib"
fi

AC_CHECK_LIB(
  vanessa_logger,
  vanessa_logger_closelog,
  :,
  AC_MSG_ERROR(
    ""
    "**********************************************************************"
    "* vanessa_socket requires the vanessa_logger generic logging library"
    "* available from"
    "* http://www.vergenet.net/linux/vanessa/ and mirrors."
    "**********************************************************************"
  ) ;\
)
AC_CHECK_LIB(
  popt,
  poptGetContext,
  AM_CONDITIONAL(PIPE_BUILD, true),
  AC_MSG_WARN(
    ""
    "**********************************************************************"
    "* vanessa_socket_pipe requires the popt options parsing library"
    "* available from ftp://ftp.rpm.org/pub/rpm/ and mirrors."
    "* vanessa_socket_pipe will _not_ be built. "
    "* Proceeding with build of libvanessa_socket."
    "**********************************************************************"
  ) ;
  AM_CONDITIONAL(PIPE_BUILD, false)
  sleep 5
)
AC_MSG_CHECKING("if stderr and stdio can be reassigned");
AC_TRY_COMPILE(
        [#include <stdio.h>],
        [stdout=fdopen(1, "a");],
        [AC_MSG_RESULT("yes")
	 AC_DEFINE(WITH_REASSIGN_IO, 1, [Can stdio and stderr be reassigned]) ],
        AC_MSG_RESULT("no")
)

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_TYPE_SIGNAL
AC_FUNC_WAIT3
AC_CHECK_FUNCS(flim)

AC_SUBST(extra_libs)
AC_SUBST(vanessa_logger_libs)
AC_SUBST(pipe_dir)

AC_OUTPUT(
debian/Makefile 
libvanessa_socket/Makefile 
vanessa_socket_pipe/Makefile 
vanessa_socket_pipe/vanessa_socket_pipe_config.h 
Makefile
libvanessa_socket2.spec
vanessa-socket.pc
INSTALL
)
