/**********************************************************************
 * options.c                                             September 1999
 * Simon Horman                                      horms@verge.net.au
 *
 * Read in command line options
 * Code based on man getopt(3), later translated to popt
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

#include "options.h"
#include "unused.h"


/***********************************************************************
 * opt_p
 * Assign an option that is a char *
 * pre: opt: option to assign
 *      value: value to copy into opt
 *      flag:  flags as per options.h
 * post: Value is copied into opt. Any existing value of opt is freed
 *       unless f is set to OPT_NOT_SET
 * return: 0 on success
 *         -1 on error
 ***********************************************************************/

static int
opt_p(char **opt, const char *value, const int flag)
{
	if (!(flag & OPT_NOT_SET) && !opt)
		free(opt);
	if (!value) {
		opt = NULL;
		return 0;
	}
	*opt = strdup(value);
	if (!*opt) {
		VANESSA_LOGGER_DEBUG_ERRNO("strdup");
		return -1;
	}
	return 0;
}

/***********************************************************************
 * opt_i
 * Assign an option that is an int
 * pre: opt: option to assign
 *      value: value to assign to opt
 *      flag:  ignored
 * post: Value is assigned to opt.
 * return: 0 on success
 *         -1 on error
 ***********************************************************************/

static int
opt_i(int *opt, const int value, const int UNUSED(flag))
{
	*opt = value;
	return 0;
}



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

int options(int argc, char **argv, options_t *opt){
  int c=0;
  char *optarg;
  poptContext context;

  const struct poptOption pop_opt[] =
  {
    {"connection_limit", 'c', POPT_ARG_STRING, NULL, 'c', NULL, NULL},
    {"debug",            'd', POPT_ARG_NONE,   NULL, 'd', NULL, NULL},
    {"help",             'h', POPT_ARG_NONE,   NULL, 'h', NULL, NULL},
    {"listen_host",      'l', POPT_ARG_STRING, NULL, 'l', NULL, NULL},
    {"listen_port",      'L', POPT_ARG_STRING, NULL, 'L', NULL, NULL},
    {"no_lookup",        'n', POPT_ARG_NONE,   NULL, 'n', NULL, NULL},
    {"outgoing_host",    'o', POPT_ARG_STRING, NULL, 'o', NULL, NULL},
    {"outgoing_port",    'O', POPT_ARG_STRING, NULL, 'O', NULL, NULL},
    {"quiet",            'q', 0,               NULL, 'q', NULL, NULL},
    {"timeout",          't', POPT_ARG_STRING, NULL, 't', NULL, NULL},
    {NULL,               0,   0,               NULL, 0,   NULL, NULL}
  };

  if(argc==0 || argv==NULL) return(0);

	if (opt_i(&opt->connection_limit, DEFAULT_CONNECTION_LIMIT,
		  OPT_NOT_SET)) {
		VANESSA_LOGGER_DEBUG("Error setting default values");
		return -1;
	}
	if (opt_i(&opt->debug, DEFAULT_DEBUG, OPT_NOT_SET)) {
		VANESSA_LOGGER_DEBUG("Error setting default values");
		return -1;
	}
	if (opt_p(&opt->listen_host, DEFAULT_LISTEN_HOST, OPT_NOT_SET)) {
		VANESSA_LOGGER_DEBUG("Error setting default values");
		return -1;
	}
	if (opt_p(&opt->listen_port, DEFAULT_LISTEN_PORT, OPT_NOT_SET)) {
		VANESSA_LOGGER_DEBUG("Error setting default values");
		return -1;
	}
	if (opt_i(&opt->no_lookup, DEFAULT_NO_LOOKUP, OPT_NOT_SET)) {
		VANESSA_LOGGER_DEBUG("Error setting default values");
		return -1;
	}
	if (opt_p(&opt->outgoing_host, DEFAULT_OUTGOING_HOST, OPT_NOT_SET)) {
		VANESSA_LOGGER_DEBUG("Error setting default values");
		return -1;
	}
	if (opt_p(&opt->outgoing_port, DEFAULT_OUTGOING_PORT, OPT_NOT_SET)) {
		VANESSA_LOGGER_DEBUG("Error setting default values");
		return -1;
	}
	if (opt_i(&opt->quiet, DEFAULT_QUIET, OPT_NOT_SET)) {
		VANESSA_LOGGER_DEBUG("Error setting default values");
		return -1;
	}
	if (opt_i(&opt->timeout, DEFAULT_TIMEOUT, OPT_NOT_SET)) {
		VANESSA_LOGGER_DEBUG("Error setting default values");
		return -1;
	}


  context= poptGetContext(
    "vanessa_socket_pipe", 
    argc, 
    (const char **) argv, 
    pop_opt, 
    0
  );

  while ((c=poptGetNextOpt(context)) >= 0){
    optarg=(char *)poptGetOptArg(context);
    switch (c){
      case 'c':
	if(!vanessa_socket_str_is_digit(optarg)){ usage(-1); }
	opt_i(&opt->connection_limit, atoi(optarg), 0);
	break;
      case 'd':
	opt_i(&opt->debug, 1, 0);
	break;
      case 'h':
	usage(0);
	break;
      case 'l':
        opt_p(&opt->listen_host, optarg, 0);
	break;
      case 'L':
        opt_p(&opt->listen_port, optarg, 0);
	break;
      case 'n':
	opt_i(&opt->no_lookup, 1, 0);
	break;
      case 'o':
        opt_p(&opt->outgoing_host, optarg, 0);
	break;
      case 'O':
        opt_p(&opt->outgoing_port, optarg, 0);
	break;
      case 'q':
        opt_i(&opt->quiet, 1, 0);
	break;
      case 't':
        if(!vanessa_socket_str_is_digit(optarg)){ usage(-1); }
	opt_i(&opt->timeout, atoi(optarg), 0);
	break;
    }
  }

  if (c < -1) {
    fprintf(
      stderr,
      "options: %s: %s\n",
      poptBadOption(context, POPT_BADOPTION_NOALIAS),
      poptStrerror(c)
    );
    usage(-1);
  }

  if(opt->outgoing_host==NULL || opt->listen_port==NULL){
    usage(-1);
  }
  if(opt->outgoing_port==NULL){
    opt->outgoing_port=opt->listen_port;
  }
  
  poptFreeContext(context);

  return(0);
}


/**********************************************************************
 * log_options
 * Log options 
 * pre: opt: options to log
 *      vl: logger to log to
 * post: opt is logged to vl
 * return: none
 **********************************************************************/

int log_options(options_t opt, vanessa_logger_t *vl){

  vanessa_logger_log(
    vl,
    LOG_DEBUG,
    "connection_limit=%d, "
    "debug=%d, "
    "listen_host=\"%s\", "
    "listen_port=\"%s\", "
    "no_lookup=%d, "
    "outgoing_host=\"%s\", "
    "outgoing_port=\"%s\", "
    "quiet=%d, "
    "timeout=%d,\n",
    opt.connection_limit,
    opt.debug,
    str_null_safe(opt.listen_host),
    str_null_safe(opt.listen_port),
    opt.no_lookup,
    str_null_safe(opt.outgoing_host),
    str_null_safe(opt.outgoing_port),
    opt.quiet,
    opt.timeout
  );

  return(0);
}


/**********************************************************************
 * usage
 * Display usage information
 * pre: exit_status: exit status to exit with
 * post: Usage information printed to stdout if exit_status=0, 
 *       stderr otherwise
 *       Exit with exit_status
 * return: does not return
 **********************************************************************/

void usage(int exit_status){
  FILE *stream;

  stream=(exit_status)?stderr:stdout;
  
  fprintf(
    stream, 
    "vanessa_socket_pipe version %s Copyright Simon Horman\n"
    "\n"
    "TCP/IP pipe based on libvanessa_socket\n"
    "\n"
    "Usage: vanessa_socket_pipe [options]\n"
    "  options:\n"
    "     -c|--connection_limit:\n"
    "                         Maximum number of connections to accept\n"
    "                         simultaneously. A value of zero sets\n"
    "                         no limit on the number of simultaneous\n"
    "                         connections.\n"
    "                         (default %d)\n"
    "     -d|--debug:         Turn on verbose debuging to stderr.\n"
    "     -h|--help:          Display this message.\n"
    "     -L|--listen_port:   Port to listen on. (mandatory)\n"
    "     -l|--listen_host:   Address to listen on.\n"
    "                         May be a hostname or an IP address.\n"
    "                         If not defined then listen on all local\n"
    "                         addresses.\n"
    "     -n|--no_lookup:     Turn off lookup of hostnames and portnames.\n"
    "                         That is, hosts must be given as IP addresses\n"
    "                         and ports must be given as numbers.\n"
    "     -O|--outgoing_port: Define a port to connect to.\n"
    "                         If not specified -l|--listen_port will be used\n"
    "     -o|--outgoing_host: Define host to connect to.\n"
    "                         May be a hostname or an IP address. (mandatory)\n"
    "     -q|--quiet:         Only log errors. Overriden by -d|--debug.\n"
    "     -t|--timeout:       Idle timeout in seconds.\n"
    "                         Value of zero sets infinite timeout.\n"
    "                         (default %d)\n"
    "\n"
    "     Notes: Default value for binary flags is off.\n"
    "            -L|--listen_port and -o|--outgoing_host must be defined.\n",
    VERSION,
    DEFAULT_CONNECTION_LIMIT,
    DEFAULT_TIMEOUT
  );

  exit(exit_status);
}
