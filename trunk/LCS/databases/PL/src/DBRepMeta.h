//#  DBRepMeta.h: Database representation of the PersistentObject::MetaData.
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

#ifndef LOFAR_PL_DBREPMETA_H
#define LOFAR_PL_DBREPMETA_H

// \file DBRepMeta.h
// Database representation of the PersistentObject::MetaData.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <PL/PLfwd.h>
#include <PL/ObjectId.h>

namespace LOFAR
{
  namespace PL
  {
    // \addtogroup PL
    // @{

    // This class is an adapter class that transforms the \e metadata
    // of a persistent object to/from an internal representation, which is
    // guaranteed to be contigous. DTL and ODBC demand that the data they
    // manipulate is contiguous.
    struct  DBRepMeta
    {
      // Define the bindings between the data members in this class and the
      // BoundIOs object of DTL.
      void bindCols(dtl::BoundIOs& cols);

      // Object-ID of the persistent object.
      ObjectId::oid_t itsOid;

      // Object-ID of the owner of the persistent object.
      ObjectId::oid_t itsOwnerOid;

      // Version number of the persistent object.
      unsigned int    itsVersionNr;
    };

    // @}

  } // namespace PL

} // namespace LOFAR

#endif
