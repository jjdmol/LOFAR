//
//  Clock.c: 
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
#include <asm/ioctl.h>

#define SPID_IOC_MAGIC 'E'

#define SPID_IOCHARDREST _IO(SPID_IOC_MAGIC, 0)
#define SPID_IOC_MAXNR 0

int main(int argc, char* argv[])
{
  int fd = open("/dev/spid0", O_RDWR);
  if (fd < 0)
  { 
    printf("Error while oppening /dev/spid0: %s\n", strerror(errno));
    return -1;
  }
  if (ioctl(fd, SPID_IOCHARDREST) == 0)
  {
    printf("Usecount is reset");
  }
  
  close(fd);
  return 0;
}

                                       


