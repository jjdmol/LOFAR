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

#ifndef LOFAR_PL_PERSISTENTOBJECT_H
#define LOFAR_PL_PERSISTENTOBJECT_H

// \file PersistentObject.h
// Interface class for persistent objects.

//# Includes
#include <PL/ObjectId.h>
#include <Common/LofarTypes.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>
#include <climits>
#include <map>
#include <iostream>

namespace LOFAR
{
  namespace PL
  {
    // \addtogroup PL
    // @{

    //# Forward Declarations
    class QueryObject;
    struct DBRepMeta;

    //
    // PersistentObject is an abstract base class (i.e. interface) for
    // persistent objects. Persistent objects are uniquely identified by their
    // ObjectId, which is stored in the so-called metadata. Apart from the
    // object-id, the metadata contain the object-id of the "owning"
    // persistent object (if any), a version number that is used to detect
    // concurrent updates of the database, and the table name of the database
    // table that is associated with this persistent object.
    //
    // PersistentObject could also act as a Virtual Proxy. This design offers
    // the possibility to use lazy initialization for large (complex)
    // objects. However, this is not implemented yet.
    //
    class PersistentObject
    {
    private:

      // Forward declaration of nested private class.
      class MetaData;


    public:

      // We will often need shared pointers to this base class, hence
      // this typedef.
      typedef boost::shared_ptr<PersistentObject> Pointer;

      // This is the type of map we're gonna use for storing the relation
      // between our attributes and the table columns.
      typedef std::map<std::string, std::string> attribmap_t;

      // This is the type of container that will hold the shared pointers
      // to the PersistentObjects that we "own".
      typedef std::vector<Pointer> POContainer;


      // Remove this instance of PersistentObject from the database.
      void erase() const;

      // Insert the PersistentObject into the database.
      // \note insert() will \e always create a new stored object. This is
      // contrary to the behaviour of save() which will only create a \e new
      // stored object if it did not already exist in the database.
      void insert() const;

      // This method will typically be used to refresh an instance of 
      // PersistentObject that already resides in memory. We will need it if
      // another process or thread changed the data in the database.
      void retrieve();
      
      // Set the data in this PersistentObject equal to the data in the
      // database belonging to the object with the specified ObjectId.
      void retrieve(const ObjectId& oid);

      // Get the instances matching the query and store the first result
      // in this object. It returns the number of matching objects.
      virtual int retrieveInPlace(const QueryObject& q,
				  int maxObjects = INT_MAX) = 0;

      // Store the PersistentObject into the database. This method will
      // typically be called by the PersistenceBroker, because at this level 
      // we lack knowledge of where to store our data.
      // save() will automatically figure out whether the PersistentObject is
      // new and thus needs to be \e inserted into the database, or is already
      // present in the database and thus needs to be \e updated.
      void save() const;

      // Update the PersistentObject into the database.
      // \note update() will \e always modify an existing stored object. 
      // Therefore, calling update() on a PersistentObject that is not
      // already present in the database is an error.
      // \throw LOFAR::PL::Exception
      void update() const;

      // Return a reference to the metadata.
      // \note This method must be \c const because it is used by insert(),
      // update(), etc. which are (logically) \c const.
      MetaData& metaData() const { return itsMetaData; }

      // Return whether this PersistentObject is in the database.
      bool isPersistent() const { return metaData().versionNr() != 0; }

      // Return whether this PersistentObject is owned by another one.
      bool isOwned() const { 
        return metaData().ownerOid() != MetaData::nullOid();
      }

      // Get the database table name that is associated with this
      // PersistentObject.
      // \note This method is only provided as a convenience as you can also
      // get the table name directly using the metadata.
      const std::string& tableName() const { return metaData().tableName(); }

      // Set the database table name that is associated with this
      // PersistentObject.
      // \note This method is only provided as a convenience as you can also
      // set the table name directly using the metadata.
      void tableName(const std::string& aName) {
        metaData().tableName() = aName; 
      }

      // Return a comma delimited list of all the table names that are
      // associated with this persistent object. In other words, tableNames()
      // will return the table name of this persistent objects and the table
      // names of its owned persistent objects.
      //
      // \todo Do we need this method? I thought I would need it when doing a
      // retrieve of a complex object that is stored across several tables,
      // but it seems I can do without. Currently, I do not see other
      // situations where you might want to use this method.
      std::string tableNames() const;

      // Return a reference to the container of "owned" PersistentObjects.
      POContainer& ownedPOs() { return itsOwnedPOs; }

      // Return a const reference to the container of "owned"
      // PersistentObjects.
      const POContainer& ownedPOs() const { return itsOwnedPOs; }

      // Return the attribute map for this PersistentObject.
      virtual const attribmap_t& attribMap() const = 0;


    protected:

      // Default constructor.
      PersistentObject() {}

      // We need a virtual destructor, cause this is our abstract base class.
      virtual ~PersistentObject() {}

      // This method \e must be called by every constructor. It must ensure
      // that the POContainer is initialized properly and that the ownership
      // relations between the PersistentObjects in this container are
      // properly set in their associated MetaData objects.
      virtual void init() = 0;

      // Convert the MetaData in our persistent object to DBRepMeta format,
      // which stores all data members contiguously in memory.
      void toDBRepMeta(DBRepMeta& dest) const;

      // Convert the data from DBRepMeta format to the MetaData format in our
      // persistent object.
      void fromDBRepMeta(const DBRepMeta& org);


    private:

      // The metadata for our persistent object will be stored in a separate
      // class MetaData. We do not want to keep these metadata inside our
      // PersistentObject, because we want to share these metadata. Hence,
      // MetaData contains reference counted pointers to the actual
      // metadata. Remember that multiple copies of a PersistentObject can
      // refer to the same data, so they should also refer to the same
      // metadata.
      class MetaData
      {
      public:

        // Default constructor. 
        MetaData() : 
          itsOid(new ObjectId()), 
          itsOwnerOid(theirNullOid),
          itsVersionNr(new uint(0)),
          itsTableName(new std::string())
        {}

        // Clone the MetaData, i.e. make a deep copy.
        // \note The cloned MetaData object is created on the heap, and 
        // ownership is transferred to the caller.
        //   MetaData* clone() const;

        // Reset the attributes to their initial values.
        void reset() const;

        // Return the shared pointer to the object-id of this
        // PersistentObject.
        // \note We return a pointer here, because we must be able to attach
        // our ObjectId pointer to an instance of a shared ObjectId object.
        boost::shared_ptr<ObjectId>& oid() { return itsOid; }

        // Return the shared pointer to the object-id of the owner of this
        // PersistentObject.
        // \note We return a pointer here, because we must be able to attach
        // our ObjectId pointer to an instance of a shared ObjectId object.
        boost::shared_ptr<ObjectId>& ownerOid() { return itsOwnerOid; }

        // Return a reference to the version number of this PersistentObject.
        uint& versionNr() const { return *itsVersionNr; }

        // Return a reference to the string holding the database table name
        // for this PersistentObject.
        std::string& tableName() { return *itsTableName; }

        // Return a shared pointer to the "global" null object-id.
        static const boost::shared_ptr<ObjectId>& nullOid() { 
          return theirNullOid; 
        }

        friend std::ostream& operator<<(std::ostream& os, const MetaData& md)
        {
          os << std::endl << "itsOid       = " << md.itsOid->get()
             << std::endl << "itsOwnerOid  = " << md.itsOwnerOid->get()
             << std::endl << "itsVersionNr = " << *md.itsVersionNr
             << std::endl << "itsTableName = " << *md.itsTableName;
          return os;
        }

      private:
  
        // \name Data members of MetaData
        // All data members of MetaData are using shared pointers. We want to
        // be able to safely copy MetaData classes. However, we do not want
        // multiple copies of the member data of MetaData, because that would
        // lead to potential inconsistencies in the metadata.

        // @{

        // One "global" null object-id, which can be used to quickly
        // initialize object-ids.  
        // \note As \c theirNullOid is created in the static initialization
        // phase, there is little we can do to detect failures. This is not a
        // real concern as ObjectId is a relatively simple object that does
        // not allocate resources other than memory.
        static const boost::shared_ptr<ObjectId> theirNullOid;

        // ObjectId is used to uniquely identify every instance of a
        // PersistentObject.
        boost::shared_ptr<ObjectId> itsOid;

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
        boost::shared_ptr<ObjectId> itsOwnerOid;
  
        // Keep track of the number of \e updates that were done on this
        // object in the database. This counter can be used to implement
        // optimistic locking. Whenever itsVersionNr differs from the
        // value in the database, changes were made to the object in the
        // database \e after we retrieved it. In that case, we cannot 
        // continue with an update, because then we would overwrite modified
        // data.
        boost::shared_ptr<uint> itsVersionNr;

        // This is the name of the database table where (part of) our object
        // will be stored. It can be set by the user, although it will usually
        // be derived from the name in the map-file that is used by the genDB
        // tools.
        boost::shared_ptr<std::string> itsTableName;

        // @}

      }; //# class MetaData


      // This method is responsible for actually erasing the \e primitive
      // data members of \c T.
      virtual void doErase() const = 0;

      // This method is responsible for actually inserting the \e primitive
      // data members of \c T.
      virtual void doInsert() const = 0;

      // This method is responsible for actually retrieving the \e primitive
      // data members of \c T. \c isOwnerOid is used to indicate whether \c
      // oid refers to the object itself or to its owner.
      virtual void doRetrieve(const ObjectId& oid, bool isOwnerOid) = 0;

      // This method is responsible for actually updating the \e primitive
      // data members of \c T.
      virtual void doUpdate() const = 0;

      // Here we keep our metadata. MetaData can be copied safely, because
      // the data members within MetaData use shared pointers.
      // \note \c itsMetaData must be mutable because it is modified by
      // logically \c const methods like insert(), update(), etc.
      mutable MetaData itsMetaData;

      // Vector of shared pointers to all the PersistentObjects that we "own".
      // A PersistentObject can contain zero or more PersistentObjects. We
      // chose, however, not to add them as member data, but to create them
      // outside of this PersistentObject and retain a shared pointer to them.
      // This is what we call "ownership", as the "contained"
      // PersistentObjects will come and go with this PersistentObject.
      POContainer itsOwnedPOs;

    }; //# class PersistentObject


    // Compare two instances of PersistentObject. PersistentObjects are
    // considered equal when their ObjectIds are equal. MetaData stores a
    // shared pointer to the ObjectId, hence we can compare the pointers.
    inline bool operator==(const PersistentObject& lhs, 
                           const PersistentObject& rhs)
    {
      return lhs.metaData().oid() == rhs.metaData().oid();
    }

    // @}

  } // namespace PL

} // namespace LOFAR

#endif
