//#  DTLBase.h: Base classes and methods for use of DTL within PL.
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

#ifndef LOFAR_PL_DTLBASE_H
#define LOFAR_PL_DTLBASE_H

#include <lofar_config.h>

#if !defined(HAVE_DTL)
#error "DTL library is required"
#endif

//# Includes
#include <PL/ObjectId.h>
#include <PL/PersistentObject.h>
/* #include <dtl/DTL.h> */
#include <dtl/DBView.h>
#include <dtl/select_iterator.h>

namespace LOFAR
{
  namespace PL
  {
    //# Forward declaration
    template<typename T> struct DBRep;

    // The BCA template class is a helper class. It provides a generic
    // interface for operator() by defining a typedef for DBRep<T>.
    template<typename T> 
    class BCA
    {
    public:
      typedef DBRep<T> DataObj;
      void operator()(dtl::BoundIOs& boundIOs, DataObj& rowbuf);
    };

    // The BPA template class is a helper class. It provides a generic
    // interface for operator() by defining a typedef for DBRep<T>.
    template<typename T> 
    class BPA
    {
    public:
      typedef DBRep<T> ParamObj ;
      void operator()(dtl::BoundIOs& boundIOs, ParamObj& paramObj);
    };

    template<>
    struct DBRep<ObjectId> {
      ObjectId::oid_t itsOid;
    };

    template<>
    struct DBRep<PersistentObject::MetaData> {
      ObjectId::oid_t itsOid;
      ObjectId::oid_t itsOwner;
      unsigned int    itsVersionNr;
    };

    template<>
    inline void 
    BCA<ObjectId>::operator()(dtl::BoundIOs& cols, DataObj& rowbuf)
    {
      cols["ObjId"] == rowbuf.itsOid;
    }

    template<>
    inline void 
    BPA<ObjectId>::operator()(dtl::BoundIOs& pos, ParamObj& param)
    {
      pos[0] == param.itsOid;
    }
    
    template<> 
    inline void 
    BCA<PersistentObject::MetaData>::operator()(dtl::BoundIOs& cols, 
                                                DataObj& rowbuf)
    {
      cols["ObjId"] == rowbuf.itsOid;
      cols["Owner"] == rowbuf.itsOwner;
      cols["VersionNr"] == rowbuf.itsVersionNr;
    }
    
  } // namespace PL
  
} // namespace LOFAR

#endif
