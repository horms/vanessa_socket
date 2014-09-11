/**********************************************************************
 * vanessa_socket_pipe.c                                    August 2000
 * Simon Horman                                      horms@verge.net.au
 *
 * Trivial user space TCP pipe that uses libvanessa_socket to
 * do all the hard work.
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

#include <errno.h>
#include <sys/socket.h>

#define CONNECT_RETRY 3
#define ERR_SLEEP 1
#define IDENT "vanessa_socket_pipe"


static size_t get_salen(const struct sockaddr *sa)
{
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
	return sa->sa_len;
#else
	if (sa->sa_family == AF_INET)
		return sizeof(struct sockaddr_in);
	else
		return sizeof(struct sockaddr_in6);
#endif
}

/**********************************************************************
 * Muriel the main function
 **********************************************************************/

int main (int argc, char **argv){
  int client;
  int server;
  struct sockaddr_storage peername;
  struct sockaddr_storage sockname;
  char *buffer;
  vanessa_logger_t *vl;
  options_t opt;
  char from_to_str[((NI_MAXHOST+NI_MAXSERV+1)*2)+2];
  char from_host_str[NI_MAXHOST];
  char to_host_str[NI_MAXHOST];
  char from_serv_str[NI_MAXSERV];
  char to_serv_str[NI_MAXSERV];
  size_t bytes_written=0;
  size_t bytes_read=0;
  int timeout=0;
  int rc;

  extern int errno;

  /*
   * Read command line options 
   */ 
  options(argc, argv, &opt);

  /*
   * Set up Logger
   */
  if((vl=vanessa_logger_openlog_filehandle(
    stderr, 
    IDENT,
    opt.debug?LOG_DEBUG:opt.quiet?LOG_ERR:LOG_INFO, 
    0
  ))==NULL){
    fprintf(stderr, "main: vanessa_logger_openlog_filehandle");
    exit(-1);
  }
  
  /*
   * Set up Logging for libvanessa_socket
   */
  vanessa_logger_set(vl);

  /*
   * Log the options 
   */
  log_options(opt, vl);

  /*
   * Set a signal handler to clean up zombies
   */
  signal(SIGCHLD,   vanessa_socket_handler_reaper);

  /* 
   * Listen on a port
   * If you want to make a TCP/IP server that forks on connect
   * then this is the function for you
   */
  if((client=vanessa_socket_server_connect(
    opt.listen_port, 
    opt.listen_host,
    opt.connection_limit, 
    (struct sockaddr *) &peername,
    (struct sockaddr *) &sockname,
    0
  ))<0){
    vanessa_logger_log(vl, LOG_DEBUG, "main: vanessa_socket_server_connect");
    vanessa_logger_log(
      vl,
      LOG_ERR,
      "Could not bind to: %s:%s\n", 
      str_null_safe(opt.listen_host), 
      str_null_safe(opt.listen_port)
    );
    exit(-1);
  }

  /*
   * Convert the address that a client connected from
   * and the address (local to this host) that the 
   * client connected to to a string for later reference
   *
   * N.B. inet_ntoa uses a global buffer which is
   *       why two calls are required, lest the second call
   *       overwrite the result of the first
   */
  printf("peername len=%d\n", get_salen((struct sockaddr *)&peername));
  rc = getnameinfo((struct sockaddr *)&peername,
                   get_salen((struct sockaddr *)&peername),
                   from_host_str, NI_MAXHOST, from_serv_str, NI_MAXSERV,
                   NI_NUMERICHOST|NI_NUMERICSERV);
  if (rc) {
        VANESSA_LOGGER_DEBUG_UNSAFE("getnameinfo peername: %s",
                                    gai_strerror(rc));
        VANESSA_LOGGER_ERR("Fatal error formatting peername");
        exit(-1);
  }
  rc = getnameinfo((struct sockaddr *)&sockname,
                   get_salen((struct sockaddr *)&sockname),
                   to_host_str, NI_MAXHOST, to_serv_str, NI_MAXSERV,
                   NI_NUMERICHOST|NI_NUMERICSERV);
  if (rc) {
        VANESSA_LOGGER_DEBUG_UNSAFE("getnameinfo sockname: %s",
                                    gai_strerror(rc));
        VANESSA_LOGGER_ERR("Fatal error formatting sockname");
        exit(-1);
  }

  strcpy(from_to_str, from_host_str);
  strcat(from_to_str, ":");
  strcat(from_to_str, from_serv_str);
  strcat(from_to_str, "->");
  strcat(from_to_str, to_host_str);
  strcat(from_to_str, ":");
  strcat(from_to_str, to_serv_str);

  /*
   * Log the session
   */ 
  vanessa_logger_log(
    vl,
    LOG_INFO,
    "Connect: %s server=%s port=%s\n",
    from_to_str,
    opt.outgoing_host,
    opt.outgoing_port
  );

  /* 
   * Talk to the real server for the client
   * IF you wish to create a TCP client then this is the call for you
   */
  if((server=vanessa_socket_client_open(
    opt.outgoing_host, 
    opt.outgoing_port, 
    opt.no_lookup
  ))<0){
    vanessa_logger_log(vl, LOG_DEBUG, "main: vanessa_socket_client_open");
    vanessa_logger_log(
      vl,
      LOG_ERR,
      "Could not connect to server: %s:%s\n",
      str_null_safe(opt.outgoing_host),
      str_null_safe(opt.outgoing_port)
    );
    sleep(ERR_SLEEP);
    exit(-1);
  }

  /* 
   * Buffer for reads and writes to the server
   */ 
  buffer = malloc(BUFFER_SIZE);
  if(!buffer) {
    vanessa_logger_log(vl, LOG_DEBUG, "main: malloc: %s", strerror(errno));
    exit(-1);
  }

  /*
   * Let the client talk to the real server
   * If you need to have file descriptors talk to each other
   * then this is the function for you.
   */
  if(vanessa_socket_pipe(
    server,
    server,
    client,
    client,
    buffer,
    BUFFER_SIZE,
    timeout,
    &bytes_written,
    &bytes_read
  )<0){
    vanessa_logger_log(vl, LOG_DEBUG, "main: vanessa_socket_pipe");
    exit(-1);
  }

  /*
   * Time to leave
   */
  vanessa_logger_log(
    vl,
    LOG_INFO,
    "Closing: %s %d %d\n", 
    from_to_str, 
    bytes_read, 
    bytes_written
  );

  close(server);
  close(client);
  vanessa_logger_unset();

  return(0);
}
