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
#include <PL/SQLTimeStamp.h>

namespace LCS
{
  namespace PL
  {
    //# Forward Declarations
    class PersistenceBroker;

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

      PersistentObject() : itsIsPersistent(false) {}

      virtual ~PersistentObject() {}

      // Insert the PersistentObject into the database.
      // \note insert() will \e always create a new stored object. This is
      // contrary to the behaviour of save() which will only create a \e new
      // stored object if it did not already exist in the database.
      virtual void insert(const PersistenceBroker* const) = 0;

      // Update the PersistentObject into the database.
      // \note update() will \e always modify an existing stored object. 
      // Therefore, calling update() on a PersistentObject that is not
      // already present in the database is an error.
      // \throw LCS::PL::Exception
      virtual void update(const PersistenceBroker* const) = 0;

      // Store the PersistentObject into the database. This method will
      // typically be called by the PersistenceBroker, because at this level 
      // we lack knowledge of where to store our data.
      // save() will automatically figure out whether the PersistentObject is
      // new and thus needs to be \e inserted into the database, or is already
      // present in the database and thus needs to be \e updated.
      virtual void save(const PersistenceBroker* const) = 0;

      // This method will typically be used to refresh an instance of 
      // PersistentObject that already resides in memory. We will need it if
      // another process or thread changed the data in the database.
      virtual void retrieve(const PersistenceBroker* const) = 0;

      // Remove this instance of PersistentObject from the database.
      virtual void erase(const PersistenceBroker* const) = 0;

      // Get the isPersistent flag.
      inline bool isPersistent() { return itsIsPersistent; }

      // Set the isPersistent flag.
      inline void isPersistent(bool b) { itsIsPersistent = b; }

    protected:
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
      // \note Strictly speaking, it is strange to say that a child class owns
      // its parent. However, from a data point of view it is very practical,
      // because the member data of the parent object appear to be part of the
      // child object.
      ObjectId itsOwnerOid;

      // The date and time of last modification of this PersistentObject in
      // the database.
      SQLTimeStamp itsTimeStamp;

      // Flag indicating whether this PersistentObject is already present in
      // the database or not.
      bool itsIsPersistent;

    };

  } // namespace PL

} // namespace LCS

#endif
