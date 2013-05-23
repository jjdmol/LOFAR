//# SharedMemory.cc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include "SharedMemory.h"

#include <ctime>
#include <fstream>

#include <Common/Exception.h>
#include <Common/SystemCallException.h>
#include <Common/LofarLogger.h>
#include <Common/Thread/Mutex.h>

using namespace std;

namespace LOFAR
{
  namespace Cobalt
  {
    // Prevent multiple threads from creating/deleting the same region at the same
    // time.
    static Mutex shmMutex;


    SharedMemoryArena::SharedMemoryArena( key_t key, size_t size, Mode mode, time_t timeout )
      :
      FixedArena(NULL, size),
      key(key),
      mode(mode),
      shmid(-1),
      preexisting(false)
    {
      time_t deadline = time(0) + timeout;
      int open_flags = 0, attach_flags = 0;

      // Check whether the size is allowed
      size_t max_shm_size = maxSize();

      if (max_shm_size > 0) {
        ASSERTSTR(size <= max_shm_size, "Requested " << size << " bytes of shared memory, but system max is " << max_shm_size << " bytes.");
      }

      // Derive the open flags based on the provided mode
      switch (mode) {
        case CREATE_EXCL:
          open_flags |= IPC_EXCL;
        case CREATE:
          open_flags |= IPC_CREAT | S_IRUSR | S_IWUSR;

          // Give others read permission, to allow other users to
          // a. access our data, and to
          // b. see our buffer in 'ipcs' and related commands.
          open_flags |= S_IRGRP | S_IROTH;

#ifdef __linux__
          // The following flags will be removed below if permissions
          // do not allow them to be used.

          // Don't let the kernel swap out the buffer.
          open_flags |= SHM_NORESERVE;

          // Get huge TLB pages for faster access.
          // NOTE: For some reason, this causes shmget to succeed but shmat to
          // fail.
          open_flags |= SHM_HUGETLB;
#endif
          break;
          case READ:
            attach_flags |= SHM_RDONLY;
            break;

          case READWRITE:
          default:
            break;
          }

      // get/create shmid handle
      for(;;) {
        try {
          // Try to open the buffer
          if (open(open_flags, attach_flags, timeout > 0))
            break;
        } catch(SystemCallException &ex) {
#ifdef __linux__
          // If we used special-priviledge flags, try again without

          if (open_flags & SHM_HUGETLB) {
            LOG_WARN("Could not obtain shared memory with SHM_HUGETLB, trying without.");
            open_flags &= ~SHM_HUGETLB;
            continue;
          }

          if (open_flags & SHM_NORESERVE) {
            LOG_WARN("Could not obtain shared memory with SHM_NORESERVE, trying without.");
            open_flags &= ~SHM_NORESERVE;
            continue;
          }
#endif
          throw;
        }

        // try again until the deadline
        if (time(0) >= deadline)
          throw TimeOutException("shared memory", THROW_ARGS);

        if (usleep(999999) < 0)
          throw SystemCallException("sleep", errno, THROW_ARGS);
      }
    }


    bool SharedMemoryArena::open( int open_flags, int attach_flags, bool timeout )
    {
      ScopedLock sl(shmMutex);

      // Check whether shm area already exists, so we know whether
      // to clean up in case of failure.
      preexisting = (shmget(key, 0, 0) >= 0 || errno != ENOENT);

      shmid = shmget( key, itsSize, open_flags );

      if (shmid == -1) {
        // No timeout means we're not keeping silent about ENOENT/ENOEXIST
        if (!timeout)
          throw SystemCallException("shmget", errno, THROW_ARGS);

        if (errno != ENOENT && errno != EEXIST)
          throw SystemCallException("shmget", errno, THROW_ARGS);
      } else {
        // attach to segment
        itsBegin = shmat( shmid, NULL, attach_flags );

        if (itsBegin != (void*)-1)
          return true; // success!

        int saved_errno = errno;

        if (!preexisting) {
          // we created the buffer, so erase it before continuing
          if (shmctl(shmid, IPC_RMID, NULL) < 0)
            throw SystemCallException("shmctl", errno, THROW_ARGS);
        }

        throw SystemCallException("shmat", saved_errno, THROW_ARGS);
      }

      return false;
    }


    size_t SharedMemoryArena::maxSize()
    {
      size_t max_shm_size = 0;

#ifdef __linux__
      // Linux provides the system limit for the shared-memory size in
      // /proc/sys/kernel/shmmax. See also 'sysctl kernel.shmmax'.
      {
        ifstream shmmax("/proc/sys/kernel/shmmax");
        shmmax >> max_shm_size;

        // If everything went ok, we got our maximum
        if (shmmax.good())
          return max_shm_size;
      }
#else
      (void)max_shm_size;
#endif

      return 0;
    }

    void SharedMemoryArena::remove( key_t key, bool quiet )
    {
      int shmid = shmget( key, 0, 0 );

      if (shmid < 0)
        // key does not exist
        return;

      if (!quiet)
        LOG_WARN_STR("Removing existing SHM area 0x" << hex << key);

      // Note: the shmid might have been removed by another thread or process,
      // so getting EINVAL is a possibility.
      if (shmctl(shmid, IPC_RMID, NULL) < 0 && errno != EINVAL)
        // failed to remove SHM
        throw SystemCallException("shmctl", errno, THROW_ARGS);

      // key existed, SHM removed
    }


    SharedMemoryArena::~SharedMemoryArena()
    {
      try {
        // detach
        if (shmdt(itsBegin) < 0)
          throw SystemCallException("shmdt", errno, THROW_ARGS);

        // destroy
        if (!preexisting && (mode == CREATE || mode == CREATE_EXCL))
          if (shmctl(shmid, IPC_RMID, NULL) < 0)
            throw SystemCallException("shmctl", errno, THROW_ARGS);

      } catch (Exception &ex) {
        LOG_ERROR_STR("Exception in destructor: " << ex);
      }
    }

  }
}

