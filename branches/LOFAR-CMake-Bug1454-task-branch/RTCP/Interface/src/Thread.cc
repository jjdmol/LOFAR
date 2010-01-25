//#  Thread.cc:
//#
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
//#  $Id: CN_Configuration.cc 14162 2009-10-01 10:57:55Z mol $

#include <lofar_config.h>
#include <Interface/Thread.h>
#include <cstdio>

namespace LOFAR {
namespace RTCP {

void Thread::setSigHandler()
{
  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags   = 0;
  sa.sa_handler = sigHandler;

  if (sigaction(SIGUSR1, &sa, 0) != 0) {
    perror("sigaction");
    exit(1);
  }
}

void Thread::sigHandler(int)
{
}

} // namespace RTCP
} // namespace LOFAR
