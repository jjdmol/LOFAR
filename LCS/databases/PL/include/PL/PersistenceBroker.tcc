//#  PersistenceBroker.tcc: inline implementation of persistence broker class.
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

#ifndef LOFAR_PL_PERSISTENCEBROKER_TCC
#define LOFAR_PL_PERSISTENCEBROKER_TCC

#include <PL/Exception.h>
#include <PL/TPersistentObject.h>
#include <PL/Collection.h>

namespace LOFAR
{
  namespace PL
  {

    template<typename T>
    void 
    PersistenceBroker::erase(const Collection<TPersistentObject<T> >& ctpo) const
    {
      // \todo This should all be done within one transaction!  We could then
      // make the PersistenceBroker responsible for beginning and ending the
      // transaction, and forward the iteration over the Collection to the
      // TPersistentObject<T>. TPersistentObject<T> would then need an extra
      // method like: template<typename T> void
      // TPersistentObject<T>::erase(const Collection<TPersistentObject<T> >&);
      typename Collection<TPersistentObject<T> >::const_iterator it;
      for(it = ctpo.begin(); it != ctpo.end(); ++it) {
        it->erase();
      }
    }

    template<typename T>
    Collection<TPersistentObject<T> >
    PersistenceBroker::retrieve(const QueryObject& query, int maxObjects)
    {
      TPersistentObject<T> tpo;
      return tpo.retrieve(query, maxObjects);
    }

    template <typename T>
    TPersistentObject<T> 
    PersistenceBroker::retrieve(const ObjectId& oid) const
    {
      TPersistentObject<T> tpo;
      tpo.retrieve(oid);
      return tpo;
    }

  } // namespace PL

} // namespace LOFAR

#endif
