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

#ifndef LOFAR_PL_TPERSISTENTOBJECT_H
#define LOFAR_PL_TPERSISTENTOBJECT_H

// \file TPersistentObject.h
// Container class for making objects persistent.

#include <lofar_config.h>

#if !defined(HAVE_DTL)
#error "DTL library is required"
#endif

//# Includes
#include <PL/PersistentObject.h>
#include <PL/PLfwd.h>
#include <dtl/BoundIO.h>
#include <climits>

namespace LOFAR 
{
  namespace PL
  {
    // \addtogroup PL
    // @{

    // This templated class acts as a surrogate container class for instances
    // of T. The container provides the functionality to make instances of T
    // persistent. 
    // \note We use the term surrogate container, because TPersistentObject
    // does not act as a real STL-like container. TPersistentObject stores
    // a \e pointer to the instance of T instead of storing a \e copy, which
    // is what STL-like containers do.
    template<typename T>
    class TPersistentObject : public PersistentObject
    {
    public:

      // We need a default constructor, because we want to be able to create
      // e.g. a Collection of TPersistentObject<T>. The default constructor
      // will create a default constructed object \c T on the free store,
      // which will be deleted when \c *this is destroyed.
      TPersistentObject() :
        itsObjPtr(new T()), itsObjShrPtr(itsObjPtr)
      {
	init();
      }

      // \c T is passed by refererence, not const reference, because this
      // documents more explicitly that \c t can be easily changed, using the
      // data() method, which also returns a reference, instead of a const
      // reference. Internally we keep a pointer to the instance of T; we need
      // a pointer because a pointer can have a null value, whereas a
      // reference cannot.
      explicit TPersistentObject(T& t) : 
	itsObjPtr(&t)
      { 
	init();
      }

      virtual ~TPersistentObject() 
      {
      }
      
      // Return a reference to the contained persistent object.
      T& data() { return *itsObjPtr; }

      // Return a const reference to the contained persistent object.
      const T& data() const { return *itsObjPtr; }

      // We need the using statement, otherwise the retrieve method in the
      // base class PersistentObject will be hidden by the retrieve method in
      // this class.
      using PersistentObject::retrieve;

      // \todo Maybe this should be done completely different. It will depend
      // on how we can best iterate over the query result. We might e.g.
      // instantiate our persistent objects using their unique object-ids
      // which were returned by the query. Then we could call the
      // retrieve(ObjectId&) member function to initialize an already existing
      // PersistentObject. We will have at least one existing
      // TPersistentObject<T> anyway, because we can only add instances of
      // TPersistentObject<T> to our Collection.
      Collection< TPersistentObject<T> > 
      retrieve(const QueryObject& query, int maxObjects=INT_MAX);

      // Get the instances matching the query and store the first result
      // in this object. It returns the number of matching objects.
      virtual int retrieveInPlace(const QueryObject& q,
				  int maxObjects = INT_MAX);

      // Return the attribute map for this TPersistentObject.
      virtual const attribmap_t& attribMap() const;

      // Initialize the attribute map for this TPersistentObject.
      static void initAttribMap();

    private:
      
      // This method will be called by every TPersistentObject constructor. It
      // ensures that the POContainer in the base class PersistentObject is
      // initialized properly and that the ownership relations between the
      // PersistentObjects in this container are properly set in their
      // associated MetaData objects.
      // \attention This method must be implemented using template
      // specialization.
      virtual void init();

      // This method is responsible for actually erasing the \e primitive
      // data members of \c T.
      virtual void doErase() const;

      // This method is responsible for actually inserting the \e primitive
      // data members of \c T.
      virtual void doInsert() const;

      // This method is responsible for actually retrieving the \e primitive
      // data members of \c T. \c isOwnerOid is used to indicate whether \c
      // oid refers to the object itself or to its owner.
      virtual void doRetrieve(const ObjectId& oid, bool isOwnerOid);

      // This method is responsible for actually erasing the \e primitive
      // data members of \c T.
      virtual void doUpdate() const;

      // Here we keep a pointer to the instance of T.
      T* itsObjPtr;

      // The boost::shared_ptr will be used to keep track of instances of T
      // that we've created ourselves. Because copying a TPersistentObject
      // implies copying the pointer to the instance of T, we must somehow
      // keep track of the number of pointers pointing to this object. When
      // the count drops to zero, boost::shared_ptr will delete the object.
      // Obviously, we do not want this behaviour for instances of T that we
      // did not create ourselves. If we're not the creator, then we're not
      // the owner, hence we should never ever delete the object! Therefore,
      // we will only transfer ownership of \c itsObjPtr to the
      // boost::shared_ptr when \e we are the creator/owner of the object that
      // \c itsObjPtr points to.
      boost::shared_ptr<T> itsObjShrPtr;

      // This map describes the mapping of the attributes of \c T to the
      // columns in the database table. If an attribute of class \c T is of a
      // non-primitive type then the column name is a surrogate that refers to
      // the class type of that attribute prepended by a "@" character. The
      // lookup for the correct column name should then be done in the \c
      // attribMap of the TPersistentObject that is associated with that
      // class.
      static attribmap_t theirAttribMap;

      // Convert the data in our persistent object class to DBRep format,
      // which stores all data members contiguously in memory.
      void toDBRep(DBRepHolder<T>& dest) const;

      // Convert the data from DBRep format to our persistent object.
      void fromDBRep(const DBRepHolder<T>& src);

      // Must be implemented by the user
      void bindCols(dtl::BoundIOs& cols, DBRep<T>& rowbuf) const;

      // Must be implemented by the user
      void toDBRep(DBRep<T>& dest) const;

      // Must be implemented by the user
      void fromDBRep(const DBRep<T>& src);

    };

    // Specializations for ObjectId.
    template<>
    void TPersistentObject<ObjectId>::initAttribMap();
    template<>
    void TPersistentObject<ObjectId>::init();
    template<>
    void TPersistentObject<ObjectId>::toDBRep(DBRepHolder<ObjectId>& dest) const;
    template<>
    void TPersistentObject<ObjectId>::fromDBRep(const DBRepHolder<ObjectId>& src);

    // @}

  } // namespace PL

} // namespace LOFAR

#include <PL/TPersistentObject.tcc>

#endif
