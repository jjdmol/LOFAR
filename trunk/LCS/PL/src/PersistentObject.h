//#  PersistentObject.h: Interface class for persistent objects.
//#
//#  Copyright (C) 2002
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

#ifndef LCS_PL_PERSISTENTOBJECT_H
#define LCS_PL_PERSISTENTOBJECT_H

//# Includes
#include <PL/ObjectId.h>
#include <boost/shared_ptr.hpp>

namespace LCS
{
  namespace PL
  {
    //
    // PersistentObject is an abstract base class (i.e. interface) for 
    // persistent objects. Persistent objects are uniquely identified by their
    // ObjectID. The date and time of their reification is stored. This
    // information is needed when a modified persistent objects needs to be
    // stored into the database.
    //
    // PersistentObject also acts as a Virtual Proxy. This design offers the 
    // possibility to use lazy initialization for large (complex) objects.
    //
    class PersistentObject
    {
    protected:

      // The meta data for our persistent object will be stored in a
      // separate struct. We do not want to keep these data inside our
      // PersistentObject, because we want to be able to share these data
      // among different copies of a PersistentObject. Remember that multiple
      // copies of a PersistentObject can refer to the same data, so they
      // should also refer to the same meta data.
      struct MetaData {

        // Default constructor. 
        MetaData() : itsOwnerOid(0), itsVersionNr(0) {}

        // ObjectId is used to uniquely identify every instance of a
        // PersistentObject.
        ObjectId itsOid;

        // Each PersistentObject may be owned by another PersistentObject. 
        // We say that an object is owned by another object, when either 
        // \li the object is contained by (it is a member of) another object,
        // or 
        // \li the object is the parent (or base class) of another object.
        //
        // In order to be able to recreate an instance of an object, we need to
        // record the "ownership" as well. The owned object will record a
        // reference to its owner, because each object will have at most
        // one owner.
        //
        // \note Strictly speaking, it is strange to say that a child class
        // owns its parent. However, from a data point of view it is very
        // practical, because the member data of the parent object appear to
        // be part of the child object.
        ObjectId itsOwnerOid;
        
        // Keep track of the number of \e updates that were done on this
        // object in the database. This counter can be used to implement
        // optimistic locking. Whenever itsVersionNr differs from the
        // value in the database, changes were made to the object in the
        // database \e after we retrieved it. In that case, we cannot 
        // continue with an update, because then we would overwrite modified
        // data.
        unsigned int itsVersionNr;

      };

      // Default constructor.
      PersistentObject() : itsMetaData(new MetaData()) {}

      // We need a virtual destructor, cause this is our abstract base class.
      virtual ~PersistentObject() {}

public:

      // Insert the PersistentObject into the database.
      // \note insert() will \e always create a new stored object. This is
      // contrary to the behaviour of save() which will only create a \e new
      // stored object if it did not already exist in the database.
      virtual void insert() const = 0;

      // Update the PersistentObject into the database.
      // \note update() will \e always modify an existing stored object. 
      // Therefore, calling update() on a PersistentObject that is not
      // already present in the database is an error.
      // \throw LCS::PL::Exception
      virtual void update() const = 0;

      // Store the PersistentObject into the database. This method will
      // typically be called by the PersistenceBroker, because at this level 
      // we lack knowledge of where to store our data.
      // save() will automatically figure out whether the PersistentObject is
      // new and thus needs to be \e inserted into the database, or is already
      // present in the database and thus needs to be \e updated.
      virtual void save() const = 0;

      // This method will typically be used to refresh an instance of 
      // PersistentObject that already resides in memory. We will need it if
      // another process or thread changed the data in the database.
      virtual void retrieve() const = 0;

      // Remove this instance of PersistentObject from the database.
      virtual void erase() const = 0;

      // Return a non-const reference to the meta data. Hey, we want to be
      // able to modify these data, while inserting, updating, etc.
      MetaData& metaData() const { return *itsMetaData; }

      // Return whether this PersistentObject is in the database.
      bool isPersistent() const { return metaData().itsVersionNr != 0; }

      // Compare two instances of PersistentObject. PersistentObjects
      // are considered equal when their ObjectIds (which are stored in
      // the struct MetaData) are equal.
      friend bool operator==(const PersistentObject& lhs, 
                             const PersistentObject& rhs)
      {
        return lhs.metaData().itsOid == rhs.metaData().itsOid;
      }

private:

      // Shared pointer to the MetaData. We do not want different copies
      // of PersistentObject to have different meta data.
      boost::shared_ptr<MetaData> itsMetaData;

    };

  } // namespace PL

} // namespace LCS

#endif
