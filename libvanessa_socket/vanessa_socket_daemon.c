/**********************************************************************
 * vanessa_socket_daemon.c                               September 1999
 * Simon Horman                                      horms@verge.net.au
 *
 * Close and fork to become a vanessa_socket_daemon.
 *
 * Notes from unix programmer faq
 * http://www.landfield.com/faqs/unix-faq/programmer/faq/
 *
 * Almost none of this is necessary (or advisable) if your daemon is being
 * started by `inetd'.  In that case, stdin, stdout and stderr are all set up
 * for you to refer to the network connection, and the `fork()'s and session
 * manipulation should *not* be done (to avoid confusing `inetd').  Only the
 * `chdir()' step remains useful.
 * 
 * vanessa_socket
 * Library to simplify handling of TCP sockets
 * Copyright (C) 1999-2008  Simon Horman <horms@verge.net.au>
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <fcntl.h>

#include "vanessa_socket.h"


/**********************************************************************
 * vanessa_socket_daemon_process
 * Close all file descriptors and fork to become a vanessa_socket_daemon.
 * Note: vanessa_socket_daemon_inetd_process should be called if the 
 * process is being run from inetd.
 **********************************************************************/

void vanessa_socket_daemon_process(void)
{
	/*
	 * `fork()' so the parent can exit, this returns control to the command
	 * line or shell invoking your program.  This step is required so that
	 * the new process is guaranteed not to be a process group leader. The
	 * next step, `setsid()', fails if you're a process group leader.
	 * vanessa_socket_daemon_become_child();
	 */
	vanessa_socket_daemon_become_child();

	/*
	 * setsid()' to become a process group and session group leader. Since a
	 * controlling terminal is associated with a session, and this new
	 * session has not yet acquired a controlling terminal our process now
	 * has no controlling terminal, which is a Good Thing for daemons.
	 */

	if (setsid() < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("setsid");
		VANESSA_LOGGER_ERR
		    ("Fatal error begoming group leader. Exiting.");
		vanessa_socket_daemon_exit_cleanly(-1);
	}

	/*
	 * fork()' again so the parent, (the session group leader), can exit.
	 * This means that we, as a non-session group leader, can never regain a
	 * controlling terminal.
	 */
	vanessa_socket_daemon_become_child();

	/* chdir() */
	vanessa_socket_daemon_inetd_process();

	/*
	 * `close()' fds 0, 1, and 2. This releases the standard in, out, and
	 * error we inherited from our parent process. We have no way of knowing
	 * where these fds might have been redirected to. Note that many daemons
	 * use `sysconf()' to determine the limit `_SC_OPEN_MAX'.  `_SC_OPEN_MAX'
	 * tells you the maximun open files/process. Then in a loop, the daemon
	 * can close all possible file descriptors. You have to decide if you
	 * need to do this or not.  If you think that there might be
	 * file-descriptors open you should close them, since there's a limit on
	 * number of concurrent file descriptors.
	 */
	vanessa_socket_daemon_close_fd();

	/* Establish new open descriptors for stdin, stdout and stderr. Even if
	 * you don't plan to use them, it is still a good idea to have them open.
	 * The precise handling of these is a matter of taste; if you have a
	 * logfile, for example, you might wish to open it as stdout or stderr,
	 * and open `/dev/null' as stdin; alternatively, you could open
	 * `/dev/console' as stderr and/or stdout, and `/dev/null' as stdin, or
	 * any other combination that makes sense for your particular daemon.
	 */

	if (open("/dev/null", O_RDONLY) < 0) {
		vanessa_socket_daemon_exit_cleanly(-1);
	}
	if ((open("/dev/console", O_WRONLY | O_APPEND) < 0) &&
			open("/dev/null", O_WRONLY | O_APPEND) < 0) {
		vanessa_socket_daemon_exit_cleanly(-1);
	}
	if ((open("/dev/console", O_WRONLY | O_APPEND) < 0) &&
			open("/dev/null", O_WRONLY | O_APPEND) < 0) {
		vanessa_socket_daemon_exit_cleanly(-1);
	}

#ifdef WITH_REASSIGN_IO
	stdout = fdopen(1, "a");
	if(!stdout) {
		vanessa_socket_daemon_exit_cleanly(-1);
	}
	stderr = fdopen(2, "a");
	if(!stderr) {
		vanessa_socket_daemon_exit_cleanly(-1);
	}
#endif /* WITH_REASSIGN_IO */
}


/**********************************************************************
 * vanessa_socket_daemon_inetd_process
 * Chdir to /
 * This is all we really need to do if our process is run from
 * inetd
 **********************************************************************/

void vanessa_socket_daemon_inetd_process(void)
{
	/*
	 * `chdir("/")' to ensure that our process doesn't keep any directory in
	 * use. Failure to do this could make it so that an administrator
	 * couldn't unmount a filesystem, because it was our current directory.
	 */
	if (chdir("/") < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("chdir");
		VANESSA_LOGGER_ERR
		    ("Fatal error changing directory to /. Exiting.");
		vanessa_socket_daemon_exit_cleanly(-1);
	}
}


/**********************************************************************
 * vanessa_socket_daemon_become_child
 * Fork and exit from parent process. When we return 
 * we are our own clild. Very incestuous.
 **********************************************************************/

void vanessa_socket_daemon_become_child(void)
{
	int status;

	status = fork();

	if (status < 0) {
		VANESSA_LOGGER_DEBUG_ERRNO("fork");
		VANESSA_LOGGER_ERR("Fatal error forking. Exiting.");
		vanessa_socket_daemon_exit_cleanly(-1);
	}
	if (status > 0) {
		vanessa_socket_daemon_exit_cleanly(0);
	}
}


/**********************************************************************
 * vanessa_socket_daemon_close_fd
 * Close all the file descriptots a process has
 **********************************************************************/

void vanessa_socket_daemon_close_fd(void)
{
	int fd;
	long max_fd;

	fflush(NULL);

	if ((max_fd = sysconf(_SC_OPEN_MAX)) < 2) {
		VANESSA_LOGGER_DEBUG_ERRNO("sysconf");
		VANESSA_LOGGER_ERR
		    ("Fatal error finding maximum file descriptors. Exiting.");

		/*
		 * don't use vanessa_socket_daemon_exit_cleanly as 
		 * vanessa_socket_daemon_close_fd is called from 
		 * vanessa_socket_daemon_exit_cleanly
		 */
		exit(-1);
	}

	for (fd = 0; fd < (int) max_fd; fd++) {
		close(fd);
	}
}


/**********************************************************************
 * vanessa_socket_daemon_setid
 * Set the userid and groupid of the process.
 * Arguments are the username or the userid as a string and 
 * the group or the groupid as a string.
 **********************************************************************/

int vanessa_socket_daemon_setid(const char *user, const char *group)
{
	uid_t uid;
	gid_t gid;
	struct passwd *pw;
	struct group *gr;

	if (vanessa_socket_str_is_digit(group)) {
		gid = (gid_t) atoi(group);
	} else {
		if ((gr = getgrnam(group)) == NULL) {
			if (errno)
				VANESSA_LOGGER_DEBUG_ERRNO("getgrnam");
			VANESSA_LOGGER_DEBUG_UNSAFE("getgrnam: error finding "
					"group \"%s\"", group);
			return (-1);
		}
		gid = gr->gr_gid;
		/*free(gr); */
	}

	if (setgid(gid)) {
		VANESSA_LOGGER_DEBUG_ERRNO("setgid");
		return (-1);
	}

	if (vanessa_socket_str_is_digit(user)) {
		uid = (uid_t) atoi(user);
	} else {
		if ((pw = getpwnam(user)) == NULL) {
			if (errno)
				VANESSA_LOGGER_DEBUG_ERRNO("getpwnam");
			VANESSA_LOGGER_DEBUG_UNSAFE("getpwnam: error finding "
					"user \"%s\"", user);
			return (-1);
		}
		uid = pw->pw_uid;
		/*free(pw); */
	}

	if (setuid(uid)) {
		VANESSA_LOGGER_DEBUG_ERRNO("setuid");
		return (-1);
	}

	VANESSA_LOGGER_DEBUG_UNSAFE("uid=%d euid=%d gid=%d egid=%d",
				    getuid(),
				    geteuid(), getgid(), getegid()
	    );

	return (0);
}



/**********************************************************************
 * vanessa_socket_daemon_exit_cleanly
 * If we get a sinal then close everthing, log it and quit
 **********************************************************************/

static int vanessa_socket_daemon_exit_cleanly_called = 0;

void vanessa_socket_daemon_exit_cleanly(int i)
{
	if (vanessa_socket_daemon_exit_cleanly_called) {
		signal(i, SIG_DFL);
		abort();
	}
	vanessa_socket_daemon_exit_cleanly_called = 1;
	/*Only log if it is a signal, not a requested exit */
	if (i > 0) {
		VANESSA_LOGGER_INFO_UNSAFE("Exiting on signal %d", i);
	}
	vanessa_socket_daemon_close_fd();
	exit((i > 0) ? 0 : i);
}


/**********************************************************************
 * vanessa_socket_str_is_digit
 * Test if a null terminated string is composed entirely of digits (0-9)
 * pre: String
 * return: 1 if string contains only digits and null terminator
 *         0 if string is NULL
 *         0 otherwise
 **********************************************************************/

int vanessa_socket_str_is_digit(const char *str)
{
	int offset;

	if (str == NULL) {
		return (0);
	}

	for (offset = strlen(str) - 1; offset > -1; offset--) {
		if (!isdigit((int) *(str + offset))) {
			break;
		}
	}

	return (offset > -1 ? 0 : 1);
}
