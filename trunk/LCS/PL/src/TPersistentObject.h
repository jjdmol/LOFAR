//#  TPersistentObject.h: one line description
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

#ifndef LCS_PL_TPERSISTENTOBJECT_H
#define LCS_PL_TPERSISTENTOBJECT_H

//# Includes
#include <PL/PersistentObject.h>

#if defined(__GNUC__) && (__GNUC__ < 3)
#else
//# Only include this file if this is not GCC version prior to 3.
#include <loki/TypeTraits.h>
#endif

namespace LCS 
{
  namespace PL
  {

    //
    // This templated class acts as a container class for instances of T.
    // The container provides the functionality to make instances of T
    // persistent. 
    //
    template<typename T>
    class TPersistentObject : public PersistentObject
    {
    public:
  

      //# Unfortunatly GCC 2.95.x does not like the Loki Typelist.h file
#if defined(__GNUC__) && (__GNUC__ < 3)
      typedef T value_type;
#else
      // Figure out whether T is \c const or not. Use a \c typedef
      // to refer to the type of \c T. This solves all kinds of nasty
      // \c const problems.
      typedef 
      typename Loki::Select<Loki::TypeTraits<T>::isConst, const T, T>::Result
      value_type;
#endif

      // Obviously, value_type& is a reference, so let's use a \c typedef
      // here too.
      typedef value_type&       reference;

      // \note T is passed as refererence, because we really want 
      // TPersistentObject to only wrap the instance of T.
      TPersistentObject(reference t): itsObject(t) {}

      virtual void insert() {}
      virtual void update() {}
      virtual void save() {}
      virtual void retrieve() {}
      virtual void erase() {}

      // Return a reference to the contained PersistentObject.
      reference value() { return itsObject; }

    private:
      // Here we keep the \e reference to the instance of T, because we want
      // to wrap the original instance of T, \e not a copy.
      reference itsObject;

    };

  }

}

#endif
