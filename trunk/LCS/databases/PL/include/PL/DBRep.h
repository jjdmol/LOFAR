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
#include <PL/DBRepMeta.h>
#include <PL/ObjectId.h>
#include <PL/PersistentObject.h>

//# Forward Declarations
// namespace dtl
// {
//   class BoundIOs;
// }

namespace LOFAR
{
  namespace PL
  {
    //# Forward Declarations

    // This class is an adapter class. It transforms the data in a
    // user-defined class \c T to/from an internal representation, which is
    // guaranteed to be contiguous. DTL and ODBC demand that the data they
    // manipulate is contiguous. Hence this adapter class.
    //
    // \note This class must be reimplemented using full class template
    // specialization, because the Persistence Layer cannot know the exact
    // layout of the user-defined class that must be transformed. 
    //
    // \attention Please make sure that your specialized class inherits from
    // DBRepMeta. This will ensure that you don't have to bother about
    // transforming the meta data, which are present in each
    // PersistentObject. Transformation of these data will be done in the base
    // class DBRepMeta.
    template <typename T>
    class DBRep : public DBRepMeta
    {
    public:
      // This method defines the bindings between the data members in the
      // database representation DBRep and the BoundIOs object of DTL. Please
      // note that the BoundIOs class requires that the data in the DBRep are
      // contiguous in memory.
      void bindCols(dtl::BoundIOs& cols);

      // Convert the data in our persistent object class to DBRep format,
      // which stores all data members contiguously in memory.
      void toDBRep(const T& src);

      // Convert the data from DBRep format to our persistent object.
      void fromDBRep(T& dest) const;

    private:
      // Enforce that this class is reimplemented by defining the default
      // constructor private.
      DBRep();
    };


    // @name Full class template specializations.
    //@{

    template<>
    class DBRep<ObjectId>
    {
    public:
      void bindCols(dtl::BoundIOs& cols) {
        cols["ObjId"] == itsOid; 
      }
      void toDBRep(const ObjectId& src) {
        itsOid = src.get(); 
      }
      void fromDBRep(ObjectId& dest) const {
        dest.set(itsOid); 
      }
    private:
      ObjectId::oid_t itsOid;
    };

    template<>
    class DBRep<PersistentObject::MetaData> : public DBRepMeta
    {
    public:
      void bindCols(dtl::BoundIOs& cols) {}
      void toDBRep(const PersistentObject::MetaData& src) {}
      void fromDBRep(PersistentObject::MetaData& dest) const {}
    };

    //@}

//     // @name Template specializations for DBRepHolder
//     //@{

//     template<>
//     class DBRepHolder<ObjectId>
//     {
//     public:
//       DBRep<ObjectId::oid_t>& rep() { return itsRep; }
//       const DBRep<ObjectId::oid_t>& rep() const { return itsRep; }
//     private:
//       ObjectId itsRep;
//     };

//     //@}

  } // namespace PL

} // namespace LOFAR

#endif
