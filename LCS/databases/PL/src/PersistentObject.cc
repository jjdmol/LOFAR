//#  PersistentObject.cc: implementation of the persistent object base class.
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

#include <PL/PersistentObject.h>
#include <PL/DBRepMeta.h>
#include <PL/Exception.h>

namespace LOFAR
{
  namespace PL
  {

    ////////////////////////////////////////////////////////////////
    //                                                            //
    //                 PersistentObject::MetaData                 //
    //                                                            //
    ////////////////////////////////////////////////////////////////

    const boost::shared_ptr<ObjectId> 
    PersistentObject::MetaData::theirNullOid(new ObjectId(0));

//     PersistentObject::MetaData* PersistentObject::MetaData::clone() const
//     {
//       MetaData* md = new MetaData();
//       md->oid().set(oid().get());
//       md->ownerOid().set(ownerOid().get());
//       md->versionNr() = versionNr();
//       return md;
//     }

    void PersistentObject::MetaData::reset() const
    {
      // We do not want to reset the owner-id. Owner-ids don't need to be
      // (re)generated; they are simply attached to an exisiting object-id, 
      // according to the relationship between owning and owned TPOs. This
      // is done in PersistentObject::init().
      itsOid->reset();
      *itsVersionNr = 0;
    }


    ////////////////////////////////////////////////////////////////
    //                                                            //
    //                      PersistentObject                      //
    //                                                            //
    ////////////////////////////////////////////////////////////////

    void PersistentObject::erase() const
    {
      POContainer::const_reverse_iterator it;
      for(it = itsOwnedPOs.rbegin(); it != itsOwnedPOs.rend(); ++it) {
	(*it)->erase();
      }
      doErase();
    }

    void PersistentObject::insert() const
    {
//       // Make a copy of the metadata first. We might need it to restore
//       // the metadata when the insert actions fails.
//       boost::shared_ptr<MetaData> md(metaData().clone());

      metaData().reset();
      doInsert();
      POContainer::const_iterator it;
      for(it = itsOwnedPOs.begin(); it != itsOwnedPOs.end(); ++it) {
	(*it)->insert();
      }
    }

    void PersistentObject::retrieve()
    {
      retrieve(*metaData().oid());
    }

    void PersistentObject::retrieve(const ObjectId& oid)
    {
      doRetrieve(oid, isOwned());
      POContainer::const_iterator it;
      for(it = itsOwnedPOs.begin(); it != itsOwnedPOs.end(); ++it) {
	(*it)->retrieve(*metaData().oid());
      }
    }

    void PersistentObject::save() const
    {
      if (isPersistent()) {
        update();
      } 
      else {
        insert();
      }
    }

    void PersistentObject::update() const
    {
      doUpdate();
      POContainer::const_iterator it;
      for(it = itsOwnedPOs.begin(); it != itsOwnedPOs.end(); ++it) {
	(*it)->update();
      }
    }

    string PersistentObject::tableNames() const
    {
      string nm(tableName());
      POContainer::const_iterator it;
      for(it = itsOwnedPOs.begin(); it != itsOwnedPOs.end(); ++it) {
        nm += "," + (*it)->tableNames();
      }
      return nm;
    }

    void PersistentObject::toDBRepMeta(DBRepMeta& dest) const
    {
      dest.itsOid = metaData().oid()->get();
      dest.itsOwnerOid = metaData().ownerOid()->get();
      dest.itsVersionNr = metaData().versionNr();
    }
    
    void PersistentObject::fromDBRepMeta(const DBRepMeta& org)
    {
      metaData().oid()->set(org.itsOid);
      metaData().ownerOid()->set(org.itsOwnerOid);
      metaData().versionNr() = org.itsVersionNr;
    }

  } // namespace PL

} // namespace LOFAR
