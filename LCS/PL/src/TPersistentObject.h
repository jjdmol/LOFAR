//#  TPersistentObject.h: container class for making objects persistent.
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
#include <PL/TPersistentObjectBase.h>
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
    class TPersistentObject : public TPersistentObjectBase<T>
    {
    public:
  
      // \c T is passed by refererence, not const reference, because this
      // documents more explicitly that \c t can be easily changed, using
      // the value() method, which also returns a reference, instead of a 
      // const reference. Internally we keep a pointer to the instance of T;
      // we need a pointer because pointer can have a null value, whereas 
      // a reference cannot.
      explicit TPersistentObject(T& t) : 
        TPersistentObjectBase<T>(t)
      { 
      }

      virtual ~TPersistentObject() 
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

      friend bool operator==(const TPersistentObject<T>& lhs, 
                             const TPersistentObject<T>& rhs)
      {
        return lhs.itsOid == rhs.itsOid;
      }
      
    private:
      // @name Methods that must be implemented using template specialization
      // The implementation of the following methods will depend on the
      // class type \c T. Therefore, there is no default implementation 
      // available. 
      // \todo Maybe we shouldn't used exceptions here, cause that will 
      // unnecessarily delay detection of the programming error until
      // runtime. We could use Loki's STATIC_CHECK macro instead.

      //@{
 
      // This method is responsible for actually erasing the \e primitive
      // data members of \c T.
      // \throw NotImplemented
      virtual void doErase(const ObjectId& poid) {
 	THROW(NotImplemented, 
	      "Method should be implemented using template specialization"); 
      }

      // This method is responsible for actually inserting the \e primitive
      // data members of \c T.
      // \throw NotImplemented
      virtual void doInsert(const ObjectId& poid) {
 	THROW(NotImplemented, 
	      "Method should be implemented using template specialization"); 
      }

      // This method is responsible for actually retrieving the \e primitive
      // data members of \c T.
      // \throw NotImplemented
      virtual void doRetrieve(const ObjectId& poid) {
 	THROW(NotImplemented, 
	      "Method should be implemented using template specialization"); 
      }

      // This method is responsible for actually erasing the \e primitive
      // data members of \c T.
      // \throw NotImplemented
      virtual void doUpdate(const ObjectId& poid) {
 	THROW(NotImplemented, 
	      "Method should be implemented using template specialization"); 
      }

      //@}

    private:

      struct DBRep {};

      void toDatabaseRep(TPersistentObject<T>::DBRep& dest) const;

      void fromDatabaseRep(const TPersistentObject<T>::DBRep& org);

    };

  }

}

#endif
