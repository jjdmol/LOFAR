// LimitExec.cc: program to limit the absolute execution time of a program
//
//  Copyright (C) 2000,2001,2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//  $Log$
//  Revision 1.1  2002/05/28 09:35:05  wierenga
//  %[BugId: 23]%
//  First version of the LimitExec program.
//
//

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

static pid_t g_child = 0;
static char* g_progname = "LimitExec";

void usage_exit(void)
{
  fprintf(stderr, "Usage: LimitExec <pos. time in seconds> <program> <args>\n");
  exit(1);
}

void killchild(int sig)
{
  int result = 0;

  /* keep compiler happy */
  sig = sig;
  
  if (kill(g_child, SIGKILL) < 0)
  {
    perror("kill");
  }

  fprintf(stderr, "%s: Process %d has exceeded time limit and has been killed.\n",
	  g_progname, g_child);
}

int main(int argc, char** argv)
{
  int seconds = 0;
  int status = 1;

  if (argc < 3) usage_exit();

  seconds = atoi(argv[1]);
  if (seconds < 0) usage_exit();

  if ((g_child = fork()) < 0)
  {
    perror("fork");
    exit(1);
  }

  if (0 == g_child)
  {
    /* exec the program with the given parameters */
    if (execvp(argv[2], &argv[2]) < 0)
    {
      perror("execvp");
      exit(1);
    }
  }
  else
  {
    /* parent */

    /* install the ALARM signal handler */
    signal(SIGALRM, killchild);

    /* start the timer */
    alarm(seconds);

    /* wait for the child to exit
     * either normally or after being killed
     * by the ALARM signal handler.
     */
    (void)wait(&status);
  }

  return status;
}
