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
#include <PL/DBRepHolder.h>
/* #include <dtl/DTL.h> */
#include <dtl/DBView.h>
#include <dtl/select_iterator.h>

namespace LOFAR
{
  namespace PL
  {
    //# Forward Declarations

    // The BCA template class is a helper class. It provides a generic
    // interface for operator() by defining a typedef for DBRepHolder<T>.
    template<typename T> 
    class BCA
    {
    public:
      typedef DBRepHolder<T> DataObj;
      void operator()(dtl::BoundIOs& boundIOs, DataObj& rowbuf)
      {
        rowbuf.rep().bindColsMeta(boundIOs);
        rowbuf.rep().bindCols(boundIOs);
      }
    };


    //@name Template specializations
    //@{

    // ObjectId is part of the meta data of a persistent object. Hence, we
    // must define a specialization, because we cannot call metaBindCols() on
    // a DBRep<ObjectId> object.
    template<>
    void BCA<ObjectId>::operator()(dtl::BoundIOs& boundIOs, DataObj& rowbuf)
    {
      rowbuf.rep().bindCols(boundIOs);
    }

//     template<>
//     class BCA<PersistentObject::MetaData>
//     {
//     public:
//       typedef DBRepMeta DataObj;
//       void operator()(dtl::BoundIOs& cols, DataObj& rowbuf)
//       {
//         cols["ObjId"] == rowbuf.itsOid;
//         cols["Owner"] == rowbuf.itsOwnerOid;
//         cols["VersionNr"] == rowbuf.itsVersionNr;
//       }
//     };

    //@}

  } // namespace PL
  
} // namespace LOFAR

#endif
