//#  ObjectCache.h: one line description
//#
//#  Copyright (C) 2002-2003
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

#ifndef LOFAR_PL_OBJECTCACHE_H
#define LOFAR_PL_OBJECTCACHE_H

//# Includes
#include <PL/PersistentObject.h>
#include <map>

namespace LOFAR
{
  namespace PL
  {
    // This class stores pointers to all objects in memory that were retrieved
    // from the database. The purpose of this cache is twofold:
    // \li It ensures that only one copy of a persistent object can exist in
    //     memory.
    // \li It reduces database I/O overhead when an object is retrieved from
    //     the database multiple times.
    // \note ObjectCache is a singleton class. We use the global friend
    //     method theObjectCache() to store the instance of ObjectCache.
    class ObjectCache
    {
    public:
      // Global function that return a reference to the ObjectCache
      // singleton. The instance of ObjectCache in stored as a static variable
      // within theObjectCache() method.
      friend ObjectCache& theObjectCache();

    private:
      // @name Singleton class.
      // Disallow default construction, copy construction and assignment.
      //@{
      ObjectCache();
      ObjectCache(const ObjectCache&);
      ObjectCache& operator=(const ObjectCache&);
      //@}

      // This map stores pairs of ObjectId and PersistentObject::Pointer.
      std::map<ObjectId, PersistentObject::Pointer> itsMap;
    };

  } // namespace PL

} // namcespace LCS

#endif
