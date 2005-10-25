//#  PersistenceBroker.h: handle save/retrieve of persistent objects.
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

#ifndef LOFAR_PL_PERSISTENCEBROKER_H
#define LOFAR_PL_PERSISTENCEBROKER_H

// \file PersistenceBroker.h
// Handle save/retrieve of persistent objects.

//# Includes
#include <PL/PLfwd.h>
#include <Common/lofar_string.h>
#include <climits>

namespace LOFAR
{
  namespace PL
  {
    // \addtogroup PL
    // @{

    // %PersistenceBroker is responsible for handling (bulk) save and retrieve 
    // operations on (a collection) of persistent objects. %PersistenceBroker 
    // "knows" in which database the data reside or must be saved. 
    // %PersistenceBroker provides transactions at the object level.
    class PersistenceBroker
    {
    public:

      // SaveMode can be used to modify the default behaviour when objects
      // are saved to the database. The default behaviour is to \e insert new
      // objects, and to \e update existing objects. However, sometimes you
      // might wish to insert a new version (that is a copy) of an exisiting
      // object into the database. One way to achieve this is to make a copy
      // of the PersistentObject, but this forces you to create a new instance
      // of PersistentObject. Another way to do this is to override the default
      // behaviour of the save method by specifying \c INSERT for the SaveMode.
      enum SaveMode { AUTOMATIC, INSERT, UPDATE };

      // Connect to the database identified by the Data Source Name aDSN, 
      // using an optional aUid as user-id and an optional aPwd as password.
      void connect(const string& aDsn, const string& aUid = "", 
                   const string& aPwd = "");

      // Erase the specified persistent object, i.e. delete it from the
      // database.
      // \throw LOFAR::PL::DeleteError
      void erase(const PersistentObject& po) const;

      // Erase all the objects in the collection. All objects will be deleted
      // from the database.
      // \throw LOFAR::PL::DeleteError
      // \todo This should all be done within one transaction!  We could then
      // make the PersistenceBroker responsible for beginning and ending the
      // transaction, and forward the iteration over the Collection to the
      // TPersistentObject<T>. TPersistentObject<T> would then need an extra
      // method like:
      // <ul>
      // <li> <tt> template<typename T> void TPersistentObject<T>::erase(const
      // Collection<TPersistentObject<T> >&); </tt>
      // </ul>
      template<typename T>
      void erase(const Collection<TPersistentObject<T> >& ctpo) const;

      // Erase all persistent objects that match the specified query.
      // \throw LOFAR::PL::DeleteError
      // \todo Not implemented yet. Requires changes to the implementation of
      // TPersistentObject as well. Currently QueryObjects can only be used
      // with data retrieval (i.e. read-only) access to the database
      void erase(const QueryObject& q) const;

      // Retrieve a collection of persistent objects, that match a specific
      // query. At most \c maxObjects are returned.
      // \throw LOFAR::PL::QueryError
      // \todo maxObjects should become part of the Query.
      template<typename T> 
      static Collection<TPersistentObject<T> > 
      retrieve(const QueryObject& q, int maxObjects = INT_MAX);

      // Save the specified persistent object. A new object will be inserted
      // into the database, an existing (already persistent) object will be 
      // updated. By specifying INSERT or UPDATE as SaveMode this default
      // behaviour can be overridden.
      // \note TPersistententObject<T> is passed non-const, because the
      // timestamp field will be modified to match the new timestamp in 
      // the database.
      void save(const PersistentObject& po, enum SaveMode sm=AUTOMATIC) const;

      // Save a collection of persistent objects. New objects will be inserted
      // into the database, existing objects will be updated. By specifying
      // INSERT or UPDATE as SaveMode this default behaviour can be overridden.
      //
      // \todo This should all be done within one transaction!  We could then
      // make the PersistenceBroker responsible for beginning and ending the
      // transaction. We might choose to forward the iteration over the
      // Collection to the TPersistentObject<T>. TPersistentObject<T> would
      // then need extra methods like:
      // <ul>
      // <li> <tt> template<typename T> void TPersistentObject<T>::save(const
      // Collection<TPersistentObject<T> >&) </tt>
      // <li> <tt> template<typename T> void
      // TPersistentObject<T>::insert(const Collection<TPersistentObject<T>
      // >&) </tt>
      // <li> <tt> template<typename T> void
      // TPersistentObject<T>::update(const Collection<TPersistentObject<T>
      // >&) </tt>
      // </ul>
      template<typename T>
      void save(const Collection<TPersistentObject<T> >& ctpo,
		enum SaveMode sm=AUTOMATIC) const;

      //############## Do we need these methods below ??? ##################

//       // Return an iterator to a collection of persistent objects that match 
//       // a specific query.
//       // \throw LOFAR::PL::QueryError
//       // \todo Should we provide this methods in PersistenceBroker, or will we
//       // only support collections??
//       template<typename T> 
//       typename Collection<TPersistentObject<T> >::iterator
//       retrieveIterator(const QueryObject& q) const;

//       // Retrieve the object associated with the given object-id.
//       // \todo Should we provide this method in PersistenceBroker?? This will
//       // clearly expose the internals of how TPersistentObject keeps track
//       // of object instances.
//       template <typename T>
//       TPersistentObject<T> retrieve(const ObjectId& oid) const;

    };

    // @}

  } // namespace PL

} // namespace LOFAR

#include <PL/PersistenceBroker.tcc>

#endif
