//#  WH_ION_Gather.cc: Blue Gene processing for 1 second of sampled data
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

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace LOFAR {
namespace RTCP {

static pthread_mutex_t	     mutex	 = PTHREAD_MUTEX_INITIALIZER;
static bool		     initialized = false;
static struct BGLPersonality BGLPersonality;


struct BGLPersonality *getBGLpersonality()
{
  pthread_mutex_lock(&mutex);

  if (!initialized) {
    initialized = true;

    int fd;

    if ((fd = open("/proc/personality", O_RDONLY)) < 0) {
      perror("open /proc/personality");
      exit(1);
    }

    if (read(fd, &BGLPersonality, sizeof BGLPersonality) < 0) {
      perror("read /proc/personality");
      exit(1);
    }

    if (close(fd) < 0) {
      perror("close /proc/personality");
      exit(1);
    }
  }

  pthread_mutex_unlock(&mutex);

  return &BGLPersonality;
}

}
}

#endif
