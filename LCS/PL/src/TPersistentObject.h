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
#include <PL/Exception.h>
#include <loki/static_check.h>
#include <loki/SmartPtr.h>
#include <iostream>

namespace LCS 
{
  namespace PL
  {

    //
    // This templated class acts as a surrogate container class for instances
    // of T. The container provides the functionality to make instances of T
    // persistent. 
    // \note We use the term surrogate container, because TPersistentObject
    // does not act as a real STL-like container. TPersistentObject stores
    // a \e pointer to the instance of T instead of storing a \e copy, which
    // is what STL-like containers do.
    //
    template<typename T>
    class TPersistentObject : public PersistentObject
    {
    public:
  
      // \c T is passed by refererence, not const reference, because this
      // documents more explicitly that \c t can be easily changed, using
      // the valu() method, which also returns a reference, instead of a 
      // const reference. Internally we keep a pointer to the instance of T;
      // we need a pointer because pointer can have a null value, whereas 
      // a reference cannot.
      explicit TPersistentObject(T& t) : 
	itsObjectPtr(&t)
      { }

      ~TPersistentObject() 
      { 
      }
      
      // Dynamically create a new TPersistentObject. It will contain an 
      // instance of a default constructed object T.
      static TPersistentObject<T>& create()
      { 
	T* anObject(new T());
	TPersistentObject<T>* aTPO(new TPersistentObject<T>(*anObject));
	aTPO->itsObjectSharedPtr(anObject);
 	return *aTPO;
      }

      // @name Methods that must be implemented using template specialization
      // The implementation of the following methods will depend on the
      // class type \c T. Therefore, there is no default implementation 
      // available. 
      // \todo Maybe we shouldn't used exceptions here, cause that will 
      // unnecessarily delay detection of the programming error until
      // runtime. We could use Loki's STATIC_CHECK macro instead.
      //@{

      // Implements PersistentObject::insert(PersistenceBroker*)
      // \throw NotImplemented
      virtual void insert(const PersistenceBroker* const b);//  {
//  	THROW(NotImplemented, 
// 	      "Method should be implemented using template specialization"); 
//   	STATIC_CHECK(0,
//  		     Method_Must_Be_Implemented_Using_Template_Specialization);
//       }

      // Implements PersistentObject::update(PersistenceBroker*)
      // \throw NotImplemented
      virtual void update(const PersistenceBroker* const b) {
 	THROW(NotImplemented, 
	      "Method should be implemented using template specialization"); 
//   	STATIC_CHECK(0,
//  		     Method_Must_Be_Implemented_Using_Template_Specialization);
      }

      // Implements PersistentObject::save(PersistenceBroker*)
      virtual void save(const PersistenceBroker* const b);

      // Implements PersistentObject::retrieve(PersistenceBroker*)
      // \throw NotImplemented
      virtual void retrieve(const PersistenceBroker* const b) {
  	THROW(NotImplemented, 
	      "Method should be implemented using template specialization"); 
//  	STATIC_CHECK(0,
//  		     Method_Must_Be_Implemented_Using_Template_Specialization);
      }

      // Implements PersistentObject::erase(PersistenceBroker*)
      // \throw NotImplemented
      virtual void erase(const PersistenceBroker* const b)  {
 	THROW(NotImplemented, 
	      "Method should be implemented using template specialization"); 
//   	STATIC_CHECK(0,
//  		     Method_Must_Be_Implemented_Using_Template_Specialization);
      }

      //@}

      // @name Public methods that should have been private
      // In cases where the persistent object \c T contains non-primitive
      // members, or is a derived class, the save() method needs to
      // recursively save all objects that are "contained" in \c T. These
      // contained objects must have a (table) reference to their "parent".
      //
      // \note "Parent" should not be interpreted as "being derived from".
      // From the data point-of-view containment of one class instance by
      // another is also considered a parent-child relation. 
      //
      // \warning Programmers should not call any of these methods. In an
      // ideal world we could have declared them private.
      //
      // \todo Do we also want to pass the TimeStamp as an argument when
      // saving the "child" classes? We might do this, because we may decide
      // that a cascading save really saves one and only one object. The fact
      // that this object is constructed from multiple "subobjects" is not 
      // really relevant.

      //@{
 
      // This method is responsible for actually erasing the \e primitive
      // data member of \c T.
      // \throw NotImplemented
      virtual void erase(const ObjectId& poid) {
 	THROW(NotImplemented, 
	      "Method should be implemented using template specialization"); 
      }

      // This method is responsible for actually retrieving the \e primitive
      // data members of \c T.
      // \throw NotImplemented
      virtual void retrieve(const ObjectId& poid) {
 	THROW(NotImplemented, 
	      "Method should be implemented using template specialization"); 
      }

      // This method is responsible for actually saving the \e primitive
      // data members of \c T.
      // \throw NotImplemented
      virtual void save(const ObjectId& poid) { 
	THROW(NotImplemented, 
	      "Method should be implemented using template specialization"); 
      }

      //@}

      // Return a reference to the contained PersistentObject.
      // \note This is a \e non-const reference, because we want to allow
      // modification of *itsObject.
      T& value() { return *itsObject; }

    private:

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

#include <PL/TPersistentObject.tcc>

#endif
