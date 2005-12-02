/**********************************************************************
 * gethostbyname.c                                        December 2005
 * Horms                                             horms@verge.net.au
 *
 * gethostbyname
 * Expose gethostbyname as a shell command 
 * Copyright (C) 1999-2005  Horms and others
 *
 * Based on code from libvanessa_socket and libvanessa_logger
 * Copyright (C) 1999-2005  Horms and others
 * http://www.vergenet.net/linux/vanessa/
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define NAME    "vanessa_gethostbyname"
#define VERSION "0.9.0"
#define COPYRIGHT "Copyright (C) 1999-2005 by Horms and others"
#define WARRANTY                                                             \
NAME " comes with ABSOLUTELY NO WARRANTY.\n"                                 \
"This is free software, and you are welcome to redistribute it\n"            \
"under certain conditions.\n"                                                \
"See the GNU Lesser General Public Licence for details."


static void version(FILE *fd) 
{
	fprintf(fd, "%s version %s\n%s\n\n%s\n",
		NAME, VERSION, COPYRIGHT, WARRANTY);
}

static void usage(int exit_status) 
{
	FILE *fd;
	
	fd = exit_status ? stderr : stdout;

	version(fd);
	fprintf(fd, "\n"
		"%s converts a hostname to an ip address\n"
		"using libc's gethostbyname\n"
		"\n"
		"Usage: %s [OPTIONS] HOST\n"
		"\n"
		"Options:\n"
		"  -h|--help:    display this message and exit\n"
		"  -v|--version: version information and exit\n"
		"\n",
		NAME, NAME);

	exit(exit_status);
}


int main(int argc, char **argv)
{
	int i;
	const char *hostname = NULL;
	struct in_addr addr;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
			usage(0);
		}
		if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v")) {
			version(stdout);
			return 0;
		}
		if (hostname) {
			usage(1);
		}
		hostname = argv[i];
	}

	if (!hostname) {
		usage(1);
	}

	if (vanessa_socket_host_in_addr(hostname, &addr, 0) < 0) {
		return -1;
	}

	printf("%s\n", inet_ntoa(addr));

	return 0;
}
