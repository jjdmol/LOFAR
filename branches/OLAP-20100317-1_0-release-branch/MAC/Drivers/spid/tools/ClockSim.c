//
//  ClockSim.c: 
//
//  Copyright (C) 2003
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

int _doExit = 0;
void signalHandler(int sig);

int main(int argc, char* argv[])
{
  signal(SIGINT,  signalHandler);
  signal(SIGTERM, signalHandler);
  
  int fd = open("/dev/spid0", O_RDWR);
  if (fd < 0)
  { 
    printf("Error while oppening /dev/spid0: %s\n", strerror(errno));
    return -1;
  }
  printf("Can be stopped by CTRL-C\n");
  while (!_doExit) 
  {
    printf("Write pulse\n");
    int result = write(fd, "Simulate", 8);
    if (result < 1)
    {
      printf("Error during write to /dev/spid0: %s(%d))\n", strerror(errno), result);
    }
    if (!_doExit) sleep(1);
  }
  close(fd);
  return 0;
}

void signalHandler(int sig)
{
  if ( (sig == SIGINT) || (sig == SIGTERM) )
    _doExit = 1;
}                                            


