//#  -*- mode: c++ -*-
//#  SharedResource.h: Locking for shared resources. Not thread safe.
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
//#  $Id$

#ifndef SHAREDRESOURCE_H_
#define SHAREDRESOURCE_H_

namespace LOFAR {
  namespace CAL {

    class SharedResource
    {
    public:
      explicit SharedResource(int maxreaders = 1, int maxwriters = 1) :
	m_semaphore(0),
	m_maxreaders(maxreaders),
	m_maxwriters(maxwriters)
      {}
      virtual ~SharedResource() {}
    
      /*@{*/
      /**
       * Lock the resource for reading or writing.
       * @return true if the locking succeeded, false otherwise.
       */
      inline bool writeLock() {
	int old = m_semaphore;
	if (m_semaphore <= 0 && m_semaphore > -m_maxwriters) m_semaphore--;
	return m_semaphore != old;
      }

      inline bool readLock() {
	int old = m_semaphore;
	if (m_semaphore >= 0 && m_semaphore <  m_maxreaders) m_semaphore++;
	return m_semaphore != old;
      }

      /*@}*/

      /*@{*/
      /**
       * Unlock the resource.
       */
      inline void writeUnlock() { m_semaphore++; if (m_semaphore > 0) m_semaphore = 0; }
      inline void readUnlock()  { m_semaphore--; if (m_semaphore < 0) m_semaphore = 0; }
      /*@}*/

      /*@{*/
      /**
       * Check whether the resource is locked for
       * reading or writing, or for reading
       * @return true if the resource is locked, false otherwise.
       */
      bool isLocked()      const { return 0 != m_semaphore; }
      bool isWriteLocked() const { return m_semaphore < 0;  }
      bool isReadLocked()  const { return m_semaphore > 0;  }

    private:
      int m_semaphore;
      int m_maxreaders;
      int m_maxwriters;
    };

  }; // namespace CAL
}; // namespace LOFAR

#endif /* SHAREDRESOURCE_H_ */

