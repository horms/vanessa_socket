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

/**********************************************************************
 * vanessa_socket_pipe
 * pipe data between two file discriptors until there is an error,
 * timeout or one or both the file descriptors are closed.
 * pre: fd_a_in:  one of the file descriptors to read from
 *      fd_a_out: one of the file descriptors to write to
 *      fd_b_in:  one of the file descriptors to read from
 *      fd_b_out: one of the file descriptors to write to
 *      buffer:   allocated buffer to read data into
 *      buffer_length: size of buffer in bytes
 *      idle_timeout:  timeout in seconds to wait for input
 *                     timout of 0 = infinite timeout
 *      return_a_read_bytes: Pointer to int where number
 *                           of bytes read from a will be recorded.
 *      return_b_read_bytes: Pointer to int where number
 *                           of bytes read from b will be recorded.
 * post: data is read from fd_a_in and written to fd_b_out and
 *       data is read from fd_b_in and written to fd_a_out.
 * return: -1 on error
 *         1 on idle timeout
 *         0 otherwise (one of fd_a_in or fd_b_in closes gracefully)
 **********************************************************************/

int vanessa_socket_pipe(
  int fd_a_in, 
  int fd_a_out, 
  int fd_b_in, 
  int fd_b_out, 
  unsigned char *buffer, 
  int buffer_length,
  int idle_timeout,
  int *return_a_read_bytes,
  int *return_b_read_bytes
){
  fd_set read_template;
  fd_set except_template;
  struct timeval timeout;
  int status;
  int bytes=0;
  
  for(;;){
    FD_ZERO(&read_template);
    FD_SET(fd_a_in, &read_template);
    FD_SET(fd_b_in, &read_template);

    FD_ZERO(&except_template);
    FD_SET(fd_a_in, &except_template);
    FD_SET(fd_b_in, &except_template);

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
        VANESSA_SOCKET_DEBUG_ERRNO("vanessa_socket_pipe: select", errno);
        return(-1);
      }
      continue;  /* Ignore EINTR */
    }
    else if(
      FD_ISSET(fd_a_in, &except_template) ||
      FD_ISSET(fd_b_in, &except_template)
    ){
      VANESSA_SOCKET_DEBUG("vanessa_socket_pipe: except_template set");
      return(-1);
    }
    else if(status==0){
      VANESSA_SOCKET_DEBUG("vanessa_socket_pipe: select returned 0");
      return(-1);
    }
    else if(FD_ISSET(fd_a_in, &read_template)){
      bytes=vanessa_socket_pipe_read_write(fd_a_in,fd_b_out,buffer,buffer_length);
      *return_a_read_bytes+=(bytes>0)?bytes:0;
      *return_a_read_bytes+=1;
    }
    else if(FD_ISSET(fd_b_in, &read_template)){
      bytes=vanessa_socket_pipe_read_write(fd_b_in,fd_a_out,buffer,buffer_length);
      *return_b_read_bytes+=(bytes>0)?bytes:0;
    }
    if(bytes<0){
      VANESSA_SOCKET_DEBUG("vanessa_socket_pipe: vanessa_socket_pipe_read_write");
      return(-1);
    }
    else if(!bytes){
      return(0);
    }
  }
}


/**********************************************************************
 * vanessa_socket_pipe_read_write
 * Read data from one file descriptor (socket) and write it to another
 * pre: in_fd: file descriptor to read from
 *      out_fd: file descriptor to write to
 *      buffer: allocated buffer to store read data in
 *      buffer_length: size of the buffer
 * post: at most buffer_length bytes are read from in_fd and written 
 *       to out_fd. 
 * return: bytes read on sucess
 *         0 on EOF
 *         -1 on error
 **********************************************************************/

int vanessa_socket_pipe_read_write(
  int in_fd, 
  int out_fd, 
  unsigned char *buffer, 
  int buffer_length
){
  int bytes;

  bytes=read(in_fd, buffer, buffer_length);
  if(bytes<=0){
    if(errno){
      VANESSA_SOCKET_DEBUG_ERRNO("vanessa_socket_pipe_read_write: read", errno);
    }
    return(bytes==0?0:-1);
  }
  if(vanessa_socket_pipe_write_bytes(out_fd, buffer, bytes)){
    VANESSA_SOCKET_DEBUG(
      "vanessa_socket_pipe_read_write: vanessa_socket_pipe_write_bytes"
    );
    return(-1);
  }

  return(bytes);
}


/**********************************************************************
 * vanessa_socket_pipe_write_bytes
 * write a n bytes of a buffer to fd
 * Pre: fd: File descriptor to write to
 *      buffer: buffer to write
 *      n: number or bytes to write
 * Return: -1 on error
 *         0 otherwise
 **********************************************************************/

int vanessa_socket_pipe_write_bytes(
  const int fd, 
  const unsigned char *buffer, 
  const ssize_t n
){
  ssize_t offset;
  ssize_t bytes_written;

  if(n==0){
   return(0);
  }

  offset=0;
  do{
    bytes_written=write(fd, buffer+offset, n-offset);
    if(bytes_written<0){
      VANESSA_SOCKET_DEBUG_ERRNO("vanessa_socket_pipe_write_bytes: write", errno);
      return(-1);
    }
    offset+=bytes_written;
  }while(offset<n);

  return(0);
}
