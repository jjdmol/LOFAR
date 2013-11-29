//#  forkexec.cc: Custom fork/exec implementation for more control
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include "forkexec.h"

#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

namespace LOFAR {
  namespace ACC {

int32 forkexec( const char *command )
{
  int status;
  pid_t pid;

  pid = fork();

  switch (pid) {
    case -1:
      // error
      return -1;

    case 0:  
      // child process

      // close all filedescriptors
      for (int f = dup(2); f > 2; --f) {
        while ( close(f) == EINTR )
          ;
      }

      execl("/bin/sh", "/bin/sh", "-c", command, static_cast<char*>(0));

      // only reached if exec fails
      _exit(1);

    default:
      // parent process
      if (waitpid(pid, &status, 0) == -1) {
        // error
        return errno;
      }

      return WEXITSTATUS(status);
  }
}


  } // namespace ACC
} // namespace LOFAR
