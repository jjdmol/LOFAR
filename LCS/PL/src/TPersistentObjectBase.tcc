//#  TPersistentObjectBase.cc: abstract base class for TPersistentObject.
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

#include <PL/TPersistentObjectBase.h>
#include <PL/Exception.h>

namespace LCS
{
  namespace PL
  {

    template<typename T>
    void 
    TPersistentObjectBase<T>::erase() const
    {
      THROW(NotImplemented, "Method is not yet implemented");
    }

    template<typename T>
    void
    TPersistentObjectBase<T>::insert() const
    {
      metaData() = MetaData();  // reset the meta data structure
      doInsert(metaData());
    }

    template<typename T>
    void
    TPersistentObjectBase<T>::retrieve() const
    {
      THROW(NotImplemented, "Method is not yet implemented");
    }

    template<typename T>
    void
    TPersistentObjectBase<T>::save() const
    {
      if (isPersistent()) {
        doUpdate(metaData());
      }
      else {
        doInsert(metaData());
      }
    }

    template<typename T>
    void
    TPersistentObjectBase<T>::update() const
    {
      doUpdate(metaData());
    }

  } // namespace PL

} // namespace LCS
