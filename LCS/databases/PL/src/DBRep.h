//#  DBRep.h: Adapter class transforming data to/from internal representation.
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

// \file DBRep.h
// Adapter class transforming data to/from internal representation.

#include <lofar_config.h>

//# Includes
#include <PL/ObjectId.h>
#include <dtl/BoundIO.h>

namespace LOFAR
{
  namespace PL
  {
    // \addtogroup PL
    // @{

    // This class is an adapter class. It transforms the data in a
    // user-defined class \c T to/from an internal representation, which is
    // guaranteed to be contiguous. DTL and ODBC demand that the data they
    // manipulate is contiguous. Hence this adapter class.
    //
    // \note This class \e must be reimplemented using full class template
    // specialization, because the Persistence Layer cannot know the exact
    // layout of the user-defined class that must be transformed.
    template <typename T> struct DBRep
    {
      // This method defines the bindings between the data members in the
      // database representation DBRep and the BoundIOs object of DTL. Please
      // note that the BoundIOs class requires that the data in the DBRep are
      // contiguous in memory.
      void bindCols(dtl::BoundIOs& cols);

    private:
      // Enforce that this class is reimplemented by defining the default
      // constructor private.
      DBRep();
    };

    // \name Full class template specializations.
    // @{
    // ObjectId is part of the metadata of a persistent object. Hence, we
    // must define a specialization.
    template<> 
    struct DBRep<ObjectId>
    {
      void bindCols(dtl::BoundIOs& cols) { cols["ObjId"] == itsOid; }
      ObjectId::oid_t itsOid;
    };
    // @}

    // @}

  } // namespace PL

} // namespace LOFAR

#endif
