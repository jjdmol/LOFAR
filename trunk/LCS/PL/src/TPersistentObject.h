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

namespace LCS 
{
  namespace PL
  {
    //# Forward Declarations
    class Query;
    template<typename T> class Collection;
    template<typename T> class TPersistentObject;

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

      // We need a default constructor, because we want to be able to
      // create e.g. a Collection of TPersistentObject<T>. The default
      // constructor of TPersistentObjectBase<T> will create a default
      // constructed object \c T on the free store, which will be deleted
      // when \c *this is destroyed.
      TPersistentObject() :
        TPersistentObjectBase<T>()
      {
      }

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
      
      static Collection<TPersistentObject<T> > 
      retrieve(const Query& query, int maxObjects)
      {
        return doRetrieve(query, maxObjects);
      }

    private:
      typedef typename PersistentObject::MetaData MetaData;

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
      virtual void doErase(MetaData& md) const 
      {
 	THROW(NotImplemented, 
	      "Method should be implemented using template specialization"); 
      }

      // This method is responsible for actually inserting the \e primitive
      // data members of \c T.
      // \throw NotImplemented
      virtual void doInsert(MetaData& md) const 
      {
 	THROW(NotImplemented, 
	      "Method should be implemented using template specialization"); 
      }

      // This method is responsible for actually retrieving the \e primitive
      // data members of \c T.
      // \throw NotImplemented
      static Collection<TPersistentObject<T> > 
      doRetrieve(const Query& query, int maxObjects)
      {
 	THROW(NotImplemented, 
	      "Method should be implemented using template specialization"); 
      }

      // This method is responsible for actually erasing the \e primitive
      // data members of \c T.
      // \throw NotImplemented
      virtual void doUpdate(MetaData& md) const 
      {
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
