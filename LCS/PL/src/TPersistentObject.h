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
#include <iostream>

// #if defined(__GNUC__) && (__GNUC__ < 3)
// #else
// //# Only include this file if this is not GCC version prior to 3.
// #include <loki/TypeTraits.h>
// #endif

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
  
      typedef T            value_type;
      typedef value_type&  reference;
      typedef value_type*  pointer;

//       // We need the default constructor, because the static create() method
//       // must be able to create such objects.
//       TPersistentObject() : 
// 	itsObject(0), isOwner(false) 
//       {}

      // \c T is passed by refererence, but internally we keep a pointer
      // to the instance of T. We need a pointer because pointer can have
      // a null value, whereas references cannot.
      explicit TPersistentObject(reference t) : 
	itsObject(&t), isOwner(false) 
      { }

      // If TPersistentObject is the owner of \c itsObject then it will be
      // deleted when TPersistentObject is destroyed.
      // \warning itsObject must be a heap object, because it will be deleted.
      ~TPersistentObject() 
      { 
	if (isOwner) delete itsObject; 
      }
      
      // Dynamically create a new TPersistentObject. It will contain an 
      // instance of a default constructed object T.
      static TPersistentObject<T>* create()
      { 
	T* anObject(new T());
	TPersistentObject<T>* aTPO(new TPersistentObject<T>(*anObject));
	aTPO->isOwner = true;
	return aTPO;
      }

      virtual void insert() {}
      virtual void update() {}
      virtual void save() {}
      virtual void retrieve() {}
      virtual void erase() {}

      // Return a reference to the contained PersistentObject.
      // \note This is a \e non-const reference, because we want to allow
      // modification of *itsObject.
      reference value() { return *itsObject; }

    private:
      // For now, let's forbid copying and assignment.
      TPersistentObject(const TPersistentObject<T>&);
      TPersistentObject<T>& operator=(const TPersistentObject<T>&);

      // Here we keep a pointer to the instance of T.
      pointer itsObject;

      // Keep track of ownership of the the instance of T. If we created it
      // dynamically (using the static create() method), we must also delete
      // it.
      bool isOwner;

    };

  }

}

#endif
