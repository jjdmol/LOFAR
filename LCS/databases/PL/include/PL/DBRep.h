//#  DBRep.h: one line description
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

#ifndef LOFAR_PL_DBREP_H
#define LOFAR_PL_DBREP_H

#include <lofar_config.h>

//# Includes
#include <PL/ObjectId.h>
#include <PL/PersistentObject.h>

namespace LOFAR
{
  namespace PL
  {
    //# Forward Declarations

    struct DBRepOid
    {
      ObjectId::oid_t itsOid;
    };

    struct DBRepMeta
    {
      ObjectId::oid_t itsOid;
      ObjectId::oid_t itsOwnerOid;
      unsigned int    itsVersionNr;
    };

    template <typename T>
    struct DBRep : public DBRepMeta
    {
      // Enforce that this struct is specialized by defining the default
      // constructor private.
    private:
      DBRep();
    };

    template<>
    struct DBRep<ObjectId> : public DBRepOid
    {
    };

    template<>
    struct DBRep<PersistentObject::MetaData> : public DBRepMeta
    {
    };

  } // namespace PL

} // namespace LOFAR

#endif
