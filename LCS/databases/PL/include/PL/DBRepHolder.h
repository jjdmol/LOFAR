//#  DBRepHolder.h: one line description
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

#ifndef LOFAR_PL_DBREPHOLDER_H
#define LOFAR_PL_DBREPHOLDER_H

#include <lofar_config.h>

//# Includes
#include <PL/DBRepMeta.h>
#include <PL/DBRep.h>
#include <dtl/BoundIO.h>

namespace LOFAR
{
  namespace PL
  {
    // This class combines the representation of the meta data of a persistent
    // object (DBRepMeta) and the representation of the data in the
    // user-defined class \c T (DBRep<T>).
    template <typename T>
    struct DBRepHolder
    {
    public:
      // A convenience typedef, so that we can address the templated DBRep<T>
      // using a "plain" data type.
      typedef DBRep<T> DBRepType;

      // Return a reference to the DBRepMeta part.
      //@{
      DBRepMeta& repMeta() { return itsRepMeta; }
      const DBRepMeta& repMeta() const { return itsRepMeta; }
      //@}

      // Return a reference to the DBRepType part.
      //@{
      DBRepType& rep() { return itsRep; }
      const DBRepType& rep() const { return itsRep; }
      //@}

      // This method 'binds' the database columns to the members of the
      // user-defined class \c T and the meta data of the accompanying
      // TPersistentObject<T>. DTL's BoundIOs class and the DBRepHolder<T>
      // class are the glue.
      void bindCols(dtl::BoundIOs& cols)
      {
        itsRepMeta.bindCols(cols);
        itsRep.bindCols(cols);
     }

    private:

      // Internal representation of the meta data of a persistent object.
      DBRepMeta itsRepMeta;

      // Internal representation of the data of a user-defined class \c T.
      DBRepType itsRep;
    };


    // @name Full class template specializations.
    //@{

    // ObjectId is part of the meta data of a persistent object. Hence, we
    // must define a specialization.
    template <>
    struct DBRepHolder<ObjectId>
    {
    public:
      typedef DBRep<ObjectId> DBRepType;
      DBRepType& rep() { return itsRep; }
      const DBRepType& rep() const { return itsRep; }
      void bindCols(dtl::BoundIOs& cols) { itsRep.bindCols(cols); }
    private:
      DBRepType itsRep;
    };

    //@}

  } // namespace PL

} // namespace LOFAR

#endif
