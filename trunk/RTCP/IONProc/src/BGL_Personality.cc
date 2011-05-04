//#  BGL_Personality.cc:
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
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#if defined HAVE_BGLPERSONALITY

#include <BGL_Personality.h>
#include <Common/SystemCallException.h>
#include <Thread/Mutex.h>
#include <Thread/Cancellation.h>

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace LOFAR {
namespace RTCP {

static Mutex                 mutex;
static bool		     initialized = false;
static struct BGLPersonality BGLPersonality;


struct BGLPersonality *getBGLpersonality()
{
  ScopedLock lock( mutex );

  if (!initialized) {
    initialized = true;

    ScopedDelayCancellation dc; // don't leak open file descriptors

    int fd;

    if ((fd = open("/proc/personality", O_RDONLY)) < 0)
      throw SystemCallException("open /proc/personality", errno, THROW_ARGS);

    if (read(fd, &BGLPersonality, sizeof BGLPersonality) < 0)
      throw SystemCallException("read /proc/personality", errno, THROW_ARGS);

    if (close(fd) < 0)
      throw SystemCallException("close /proc/personality", errno, THROW_ARGS);
  }

  return &BGLPersonality;
}

}
}

#endif
