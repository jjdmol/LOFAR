/****************************************************************************/
/* ZEPTOOS:zepto-info */
/*     This file is part of ZeptoOS: The Small Linux for Big Computers.
 *     See www.mcs.anl.gov/zeptoos for more information.
 */
/* ZEPTOOS:zepto-info */
/* */
/* ZEPTOOS:zepto-fillin */
/*     $Id$
 *     ZeptoOS_Version: 1.2
 *     ZeptoOS_Heredity: FOSS_ORIG
 *     ZeptoOS_License: GPL
 */
/* ZEPTOOS:zepto-fillin */
/* */
/* ZEPTOOS:zepto-gpl */
/*      Copyright: Argonne National Laboratory, Department of Energy,
 *                 and UChicago Argonne, LLC.  2004, 2005, 2006, 2007
 *      ZeptoOS License: GPL
 * 
 *      This software is free.  See the file ZeptoOS/misc/license.GPL
 *      for complete details on your rights to copy, modify, and use this
 *      software.
 */
/* ZEPTOOS:zepto-gpl */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <bglpersonality.h>

static void __zoid_signal_handler(int signo);

static int server_sock;

/*
 * Function run at startup.  Just installs the signal handler that zoid
 * will later trigger, and a server-side unix domain socket.
 */
void __zoid_ciod_startup(void) __attribute__((constructor));
void __zoid_ciod_startup(void)
{
    struct sigaction sa;
    struct sockaddr_un addr;

    sa.sa_handler = __zoid_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, NULL))
	perror("installing signal handler");

    if (!(server_sock = socket(PF_UNIX, SOCK_STREAM, 0)))
    {
	perror("create unix domain socket");
	return;
    }
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/var/tmp/zoid.socket");
    if (bind(server_sock, (struct sockaddr*)&addr, sizeof(addr)))
    {
	perror("bind /var/tmp/zoid.socket");
	return;
    }
    if (listen(server_sock, 5) < 0)
    {
	perror("listen");
	return;
    }
}

static void __zoid_signal_handler(int signo)
{
    BGLPersonality personality;
    int fd;
    FILE* file;
    char buffer[1024];
    int inode_7000 = -1, fd_7000 = -1, inode_8000 = -1, fd_8000 = -1;
    int fds[2];
    DIR* dir;
    struct dirent *de;
    struct msghdr msg = {0};
    struct cmsghdr *cmsg;
    char msgbuf[CMSG_SPACE(sizeof(fds))];
    uid_t uid;
    int transfer_sock;
    struct iovec iov;
    char tmp;
    int service_node;

    /* Get service node's IP address from personality.  */
    if ((fd = open("/proc/personality", O_RDONLY)) < 0)
    {
	perror("open /proc/personality");
	return;
    }
    if (read(fd, &personality, sizeof(personality)) != sizeof(personality))
    {
	perror("read personality");
	return;
    }
    close(fd);
    service_node = (personality.serviceNode? personality.serviceNode :
		    personality.NFSServer);

    /* Find the inodes of port 7000 and 8000 connections to the service node,
       used by CIOD to receive commands and forward stdout/stderr.  */
    if (!(file = fopen("/proc/net/tcp", "r")))
    {
	perror("open /proc/net/tcp");
	return;
    }
    while (fgets(buffer, sizeof(buffer), file) == buffer)
    {
	int local_port, remote_address, inode;

	if (sscanf(buffer,
		   "%*d: %*x:%x %x:%*x %*x %*x:%*x %*x:%*x %*x %*d %*d %d",
		   &local_port, &remote_address, &inode) == 3 &&
	    remote_address == service_node)
	{
	    if (local_port == 7000 && inode != 0)
		inode_7000 = inode;
	    else if (local_port == 8000 && inode != 0)
		inode_8000 = inode;
	}
    }
    fclose(file);

    if (inode_7000 < 0 || inode_8000 < 0)
    {
	fprintf(stderr, "Socket connection to the service node not found!\n");
	return;
    }

    /* Now locate the file descriptors that own these sockets.
       We need to switch back to root for a second to do that...  */

    uid = geteuid();
    if (seteuid(0) < 0)
    {
	perror("seteuid");
	return;
    }

    if (!(dir = opendir("/proc/self/fd")))
    {
	perror("open /proc/self/fd");
	return;
    }
    while ((de = readdir(dir)))
    {
	char buf2[1024];
	int len;

	sprintf(buffer, "/proc/self/fd/%s", de->d_name);

	if ((len = readlink(buffer, buf2, sizeof(buf2))) > 0 &&
	    len < sizeof(buf2))
	{
	    buf2[len] = '\0';
	    if (strncmp(buf2, "socket:", strlen("socket:")) == 0)
	    {
		int inode;
		char* endbuf;

		inode = strtol(buf2 + strlen("socket:["), &endbuf, 10);
		if (inode == inode_7000)
		    fd_7000 = strtol(de->d_name, NULL, 10);
		else if (inode == inode_8000)
		    fd_8000 = strtol(de->d_name, NULL, 10);
	    }
	}
    }
    closedir(dir);

    /* Switch again to user...  */
    if (seteuid(uid) < 0)
    {
	perror("seteuid");
	return;
    }

    if (fd_7000 < 0 || fd_8000 < 0)
    {
	fprintf(stderr, "Socket connection to the service node not found!\n");
	return;
    }

    if ((transfer_sock = accept(server_sock, NULL, 0)) < 0)
    {
	perror("accept");
	return;
    }

    /* This magic is taken from cmsg(3).  */
    iov.iov_base = &tmp;
    iov.iov_len = 1;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = msgbuf;
    msg.msg_controllen = sizeof(msgbuf);
    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fds));
    fds[0] = fd_7000;
    fds[1] = fd_8000;
    memcpy(CMSG_DATA(cmsg), fds, sizeof(fds));
    msg.msg_controllen = cmsg->cmsg_len;

    if (sendmsg(transfer_sock, &msg, 0) < 0)
    {
	perror("sendmsg");
	return;
    }

    close(transfer_sock);

    kill(getpid(), SIGSTOP);
}
