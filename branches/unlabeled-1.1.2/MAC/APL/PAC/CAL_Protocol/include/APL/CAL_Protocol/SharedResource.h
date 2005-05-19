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

namespace CAL
{
  class SharedResource
  {
  public:
    SharedResource() : m_semaphore(0) {}
    virtual ~SharedResource() {}
    
    /*@{*/
    /**
     * Lock the resource for reading or writing.
     * @return true if the locking succeeded, false otherwise.
     */
    bool writeLock() { if (m_semaphore == 0) m_semaphore--; return m_semaphore == -1; }
    bool readLock() { if (m_semaphore >= 0) m_semaphore++; return m_semaphore > 0; }
    /*@}*/

    /*@{*/
    /**
     * Unlock the resource.
     */
    void writeUnlock() { m_semaphore = 0; }
    void readUnlock() { m_semaphore--; if (m_semaphore < 0) m_semaphore = 0; }
    /*@}*/

    /**
     * Check whether the resource is locked.
     * @return true if the resource is locked, false otherwise.
     */
    bool isLocked() const { return 0 == m_semaphore; }

  private:
    int m_semaphore;
  };
};

#endif /* SHAREDRESOURCE_H_ */

