//#  TPersistentObjectBase.h: abstract base class for TPersistentObject.
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

#ifndef LCS_PL_TPERSISTENTOBJECTBASE_H
#define LCS_PL_TPERSISTENTOBJECTBASE_H

//# Includes
#include <PL/PersistentObject.h>
#include <loki/SmartPtr.h>
#include <iostream>

namespace LCS 
{
  namespace PL
  {

    //
    // This templated class acts as an abstract base class for the templated
    // class TPersistentObject.
    //
    template<typename T>
    class TPersistentObjectBase : public PersistentObject
    {
    public:
      // Implements PersistentObject::erase(PersistenceBroker*)
      virtual void erase(const PersistenceBroker* const b);

      // Implements PersistentObject::insert(PersistenceBroker*)
      virtual void insert(const PersistenceBroker* const b);

      // Implements PersistentObject::retrieve(PersistenceBroker*)
      virtual void retrieve(const PersistenceBroker* const b);

      // Implements PersistentObject::save(PersistenceBroker*)
      virtual void save(const PersistenceBroker* const b);

      // Implements PersistentObject::update(PersistenceBroker*)
      virtual void update(const PersistenceBroker* const b);

      // Return a reference to the contained persistent object.
      // \note This is a \e non-const reference, because we want to allow
      // modification of *itsObject.
      T& value() { return *itsObject; }

    protected:
  
      // \c T is passed by refererence, not const reference, because it
      // explicitly shows that \c t can be easily changed, using the value()
      // method, which also returns a reference, instead of a const reference.
      // Internally we keep a pointer to the instance of T; we need a pointer
      // because pointer can have a null value, whereas a reference cannot.
      explicit TPersistentObjectBase(T& t) : 
	itsObjectPtr(&t)
      {
      }

      virtual ~TPersistentObjectBase() 
      { 
      }
      
    private:

      // This method is responsible for actually erasing the \e primitive
      // data members of \c T.
      virtual void doErase(const ObjectId& poid) = 0;

      // This method is responsible for actually inserting the \e primitive
      // data members of \c T.
      virtual void doInsert(const ObjectId& poid) = 0;

      // This method is responsible for actually retrieving the \e primitive
      // data members of \c T.
      virtual void doRetrieve(const ObjectId& poid) = 0;

      // This method is responsible for actually erasing the \e primitive
      // data members of \c T.
      virtual void doUpdate(const ObjectId& poid) = 0;

      // Here we keep a pointer to the instance of T.
      T* itsObjectPtr;

      // The Loki::SmartPtr will be used to keep track of instances of T
      // that we've created ourselves. Because copying a TPersistentObject 
      // implies copying the pointer to the instance of T, we must somehow
      // keep track of the number of pointers pointing to this object. When
      // the count drops to zero, Loki::SmartPtr will delete the object.
      // Obviously, we do not want this behaviour for instances of T that we
      // did not create ourselves. If we're not the creator, then we're not
      // the owner, hence we should never ever delete the object! Therefore,
      // we will only transfer ownership of itsObjectPtr to the Loki::SmartPtr
      // when \e we are the creator/owner of the object that itsObjectPtr
      // points to.
      Loki::SmartPtr<T> itsObjectSharedPtr;

    };

  }

}

#include <PL/TPersistentObjectBase.tcc>

#endif
