/**********************************************************************
 * vanessa_socket_pipe.c                                  November 1999
 * Horms                                             horms@vergenet.net
 *
 * Functions to pipe data between two open file descriptors (sockets)
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

#include "vanessa_socket.h"
#include "vanessa_socket_logger.h"


/**********************************************************************
 * vanessa_socket_pipe_fd_read
 * Read bytes from fd
 * Intended to be passed to vanessa_socket_pipe_*func()
 * pre: fd: file descriptor to read bytes from
 *      buf: buffer to read bytes into
 *      count: maximum number of bytes to read
 *      data: not used
 * post: A maximum of count bytes are read into buf from fd using read(2)
 *       If an error occurs the errno is read and logged using the
 *       installed logger. See vanessa_socket_logger_set().
 * return: Number of bytes read (may be zero)
 *         -1 on error
 **********************************************************************/

ssize_t vanessa_socket_pipe_fd_read(int fd, void *buf, size_t count, 
                                    void *data){
  int bytes;

  bytes=read(fd, buf, count);

  if(bytes<=0){
    if(errno){
      VANESSA_SOCKET_DEBUG_ERRNO("read");
    }
    return(bytes==0?0:-1);
  }

  return(bytes);
}


/**********************************************************************
 * vanessa_socket_pipe_fd_write
 * Write bytes to fd
 * Intended to be passed to vanessa_socket_pipe_*func()
 * pre: fd: file descriptor to write bytes to
 *      buf: buffer to write bytes from
 *      count: maximum number of bytes to write
 *      data: not used
 * post: A maximum of count bytes are written to fd from buf using write(2)
 *       If an error occurs the errno is read and logged using the
 *       installed logger. See vanessa_socket_logger_set().
 * return: Number of bytes read (may be zero)
 *         -1 on error
 **********************************************************************/

ssize_t vanessa_socket_pipe_fd_write(int fd, const void *buf, size_t count, 
                                     void *data){
  int bytes;

  bytes=write(fd, buf, count);

  if(bytes<=0){
    if(errno){
      VANESSA_SOCKET_DEBUG_ERRNO("write");
    }
    return(bytes==0?0:-1);
  }

  return(bytes);
}


/**********************************************************************
 * vanessa_socket_pipe_func
 * pipe data between two pairs of file descriptors until there is an 
 * error, * timeout or one or both the file descriptors are closed.
 * pre: rfd_a: One read file descriptor
 *      wfd_a: One write file descriptor
 *      rfd_b: The other read file descriptor
 *      wfd_b: The other write file descriptor
 *      buffer:   allocated buffer to read data into
 *      buffer_length: size of buffer in bytes
 *      idle_timeout:  timeout in seconds to wait for input
 *                     timeout of 0 = infinite timeout
 *      return_a_read_bytes: Pointer to int where number
 *                           of bytes read from a will be recorded.
 *      return_b_read_bytes: Pointer to int where number
 *                           of bytes read from b will be recorded.
 *      read_func: Function to use for low level reading.
 *      write_func: Function to use for low level writing.
 *      fd_a_data: opaque data, relating to rfd_a and wfd_a that is passwd
 *                to read_func and write_func
 *      fd_b_data: opaque data, relating to rfd_b and wfd_b that is passwd
 *                to read_func and write_func
 * post: data is read from rfd_a and written to wfd_b and read
 *       from rfd_b and written to wfd_a.
 * return: -1 on error
 *         1 on idle timeout
 *         0 otherwise (one of the file desciptors closes gracefully)
 **********************************************************************/

int vanessa_socket_pipe_func(
  int rfd_a,
  int wfd_a,
  int rfd_b,
  int wfd_b,
  unsigned char *buffer, 
  int buffer_length,
  int idle_timeout,
  int *return_a_read_bytes,
  int *return_b_read_bytes,
  ssize_t (*read_func)(int fd, void *buf, size_t count, void *data),
  ssize_t (*write_func)(int fd, const void *buf, size_t count, void *data),
  void *fd_a_data,
  void *fd_b_data
){
  fd_set read_template;
  fd_set except_template;
  struct timeval timeout;
  int status;
  int bytes=0;
  
  for(;;){
    FD_ZERO(&read_template);
    FD_SET(rfd_a, &read_template);
    FD_SET(rfd_b, &read_template);

    FD_ZERO(&except_template);
    FD_SET(rfd_a, &except_template);
    FD_SET(rfd_b, &except_template);

    timeout.tv_sec=idle_timeout;
    timeout.tv_usec=0;

    status=select(
      FD_SETSIZE,
      &read_template,
      NULL,
      &except_template,
      idle_timeout?&timeout:NULL
    );
    if(status<0){
      if(errno!=EINTR){
        VANESSA_SOCKET_DEBUG_ERRNO("select");
        return(-1);
      }
      continue;  /* Ignore EINTR */
    }
    else if(
      FD_ISSET(rfd_a, &except_template) ||
      FD_ISSET(rfd_b, &except_template)
    ){
      VANESSA_SOCKET_DEBUG("except_template set");
      return(-1);
    }
    else if(status==0){
      VANESSA_SOCKET_DEBUG("select returned 0");
      return(-1);
    }
    else if(FD_ISSET(rfd_a, &read_template)){
      bytes=vanessa_socket_pipe_read_write_func(rfd_a, wfd_b, buffer, 
        buffer_length, read_func, write_func, fd_a_data, fd_b_data);
      *return_a_read_bytes+=(bytes>0)?bytes:0;
    }
    else if(FD_ISSET(rfd_b, &read_template)){
      bytes=vanessa_socket_pipe_read_write_func(rfd_b, wfd_a, buffer, 
        buffer_length, read_func, write_func, fd_b_data, fd_a_data);
      *return_b_read_bytes+=(bytes>0)?bytes:0;
    }
    if(bytes<0){
      VANESSA_SOCKET_DEBUG("vanessa_socket_pipe_read_write");
      return(-1);
    }
    else if(!bytes){
      return(0);
    }
  }
}


/**********************************************************************
 * vanessa_socket_pipe 
 * Moved to a macro defined elsewhere
 **********************************************************************/


/**********************************************************************
 * vanessa_socket_pipe_read_write_func
 * Read data from one file io_t and write to another
 * pre: rfd: file descriptor to read from
 *      wfd: file descriptor to write to
 *      buffer: allocated buffer to store read data in
 *      buffer_length: size of the buffer
 *      read_func: function to use for low level reading
 *      write_func: function to use for low level writing
 *      rfd_data: opaque data, relating to rfd, to pass to read_func
 *      wfd_data: opaque data, relating to wfd, to pass to write_func
 * post: at most buffer_length bytes are read from in_fd and written 
 *       to out_fd. 
 * return: bytes read on success
 *         0 on EOF
 *         -1 on error
 **********************************************************************/

int vanessa_socket_pipe_read_write_func(
  int rfd, 
  int wfd, 
  unsigned char *buffer, 
  int buffer_length,
  ssize_t (*read_func)(int fd, void *buf, size_t count, void *data),
  ssize_t (*write_func)(int fd, const void *buf, size_t count, void *data),
  void *rfd_data,
  void *wfd_data
){
  int bytes;

  bytes=read_func(rfd, buffer, buffer_length, rfd_data);
  if(bytes<=0){
    if(errno){
      VANESSA_SOCKET_DEBUG("vanessa_socket_io_read");
    }
    return(bytes==0?0:-1);
  }
  if(vanessa_socket_pipe_write_bytes_func(wfd, buffer, bytes, write_func, 
                                          wfd_data)){
    VANESSA_SOCKET_DEBUG("vanessa_socket_pipe_write_bytes");
    return(-1);
  }

  return(bytes);
}


/**********************************************************************
 * vanessa_socket_pipe_read_write
 * Moved to a macro defined elsewhere
 **********************************************************************/


/**********************************************************************
 * vanessa_socket_pipe_write_bytes_func
 * write a n bytes of a buffer to fd
 * Pre: fd: file descriptor to write to
 *      buffer: buffer to write
 *      n: number or bytes to write
 *      write_func: function to use for low level writing
 *      fd_data: opaque data relating to fd, to pass to write_func
 * Return: -1 on error
 *         0 otherwise
 **********************************************************************/

int vanessa_socket_pipe_write_bytes_func(
  int fd,
  const unsigned char *buffer, 
  const ssize_t n,
  ssize_t (*write_func)(int fd, const void *buf, size_t count, void *data), 
  void *fd_data
){
  ssize_t offset;
  ssize_t bytes_written;

  if(n==0){
    return(0);
  }

  offset=0;
  do{
    bytes_written=write_func(fd, buffer+offset, n-offset, fd_data);
    if(bytes_written<0){
      VANESSA_SOCKET_DEBUG_ERRNO("write_func");
      return(-1);
    }
    offset+=bytes_written;
  }while(offset<n);

  return(0);
}


/**********************************************************************
 * vanessa_socket_pipe_write_bytes
 * Moved to a macro defined elsewhere
 **********************************************************************/



