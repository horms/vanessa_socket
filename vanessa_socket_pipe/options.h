/**********************************************************************
 * options.h                                                   May 1999
 * Simon Horman                                      horms@verge.net.au
 *
 * Read in command line options
 * Code based on man getopt(3), later translated to libpopt.
 *
 * vanessa_socket_pipe
 * Trivial TCP/IP pipe based on libvanessa_socket
 * Copyright (C) 1999-2008  Simon Horman <horms@verge.net.au>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 **********************************************************************/

#ifndef OPT_STIX
#define OPT_STIX

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <popt.h>
#include <vanessa_socket.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "vanessa_socket_pipe_config.h"
#endif

#define BUFFER_SIZE 4096

#define DEFAULT_CONNECTION_LIMIT 0
#define DEFAULT_DEBUG            0
#define DEFAULT_LISTEN_HOST      NULL
#define DEFAULT_LISTEN_PORT      NULL
#define DEFAULT_NO_LOOKUP        0
#define DEFAULT_OUTGOING_HOST    NULL
#define DEFAULT_OUTGOING_PORT    NULL
#define DEFAULT_TIMEOUT          1800 /*in seconds*/
#define DEFAULT_QUIET            0

typedef struct {
  int             connection_limit;
  int             debug;
  char            *listen_host;
  char            *listen_port;
  int             no_lookup;
  char            *outgoing_host;
  char            *outgoing_port;
  int             quiet;
  int             timeout;
} options_t;

/*Flag values for options()*/
#define OPT_NOT_SET     0x40 /*Option is not set, don't free*/


/**********************************************************************
 * options
 * Read in command line options
 * pre: argc: number or elements in argv
 *      argv: array of strings with command line-options
 *      opt:  pointer to options structure to fill in
 * post: global opt is seeded with values according to argc and argv
 * return: 0 on success
 *         -1 on error
 **********************************************************************/

int options(int argc, char **argv, options_t *opt);


/**********************************************************************
 * log_options
 * Log options 
 * pre: opt: options to log
 *      vl: logger to log to
 * post: opt is logged to vl
 * return: none
 **********************************************************************/

int log_options(options_t opt, vanessa_logger_t *vl);


/**********************************************************************
 * usage
 * Display usage information
 * pre: exit_status: exit status to exit with
 * post: Usage information printed to stdout if exit_status=0, 
 *       stderr otherwise
 *       Exit with exit_status
 * return: does not return
 **********************************************************************/

void usage(int exit_status);


/**********************************************************************
 * str_null_safe
 * return a pinter to a sane string if string is NULL
 * So we can print NULL strings safely
 * pre: string: string to test
 * return: string is if it is not NULL
 *         STR_NULL otherwise
 *
 * 8 bit clean
 **********************************************************************/

#define str_null_safe(string) \
  (string==NULL)?"(null)":string


#endif
