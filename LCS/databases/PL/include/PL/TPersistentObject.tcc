//#  TPersistentObject.tcc: inline implementation of persistent object class.
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

#ifndef LOFAR_PL_TPERSISTENTOBJECT_TCC
#define LOFAR_PL_TPERSISTENTOBJECT_TCC

#include <PL/TPersistentObject.h>
#include <PL/DTLBase.h>
#include <Common/Debug.h>

namespace LOFAR
{
  namespace PL
  {

    template<typename T>
    Collection< TPersistentObject<T> > 
    TPersistentObject<T>::retrieve(const Query& query, int maxObjects)
    {
      typedef dtl::DBView< DBRep<T> >  DBViewType;

      Collection< TPersistentObject<T> > ctpo;

      DBViewType view(tableName(), BCA<T>(), query.getSql());
      typename DBViewType::select_iterator iter = view.begin();

      for (int nr = 0; iter != view.end() && nr < maxObjects; ++iter, ++nr) {
	TPersistentObject<T> tpo;
	tpo.tableName (tableName());
	tpo.fromDatabaseRep(*iter);
        // If the object T is spread among several tables we must call
        // retrieve() in order to get the data from all the tables. Otherwise,
        // we will miss the data for the "owned" POs. This really isn't very
        // efficient! However, currently there is no way to do things
        // better. We need a way to generate a better select query for this.
        if (!tpo.ownedPOs().empty()) {
          tpo.retrieve();
        }
	ctpo.add(tpo);
      }
      return ctpo;
    }


    template<typename T>
    void TPersistentObject<T>::doErase() const
    {
      typedef dtl::DBView< DBRep<ObjectId> > DBViewType;
      DBViewType view(tableName(), BCA<ObjectId>());
      DBViewType::delete_iterator iter = view;

      // setup the selection parameters
      DBRep<ObjectId> rec;
      rec.itsOid = metaData().oid()->get();

      // delete this record
      *iter = rec;

      // Once we've reached here, the update was successful.
      // Reset the meta data structure.
      metaData().reset();
    }


    template<typename T>
    void TPersistentObject<T>::doInsert() const
    {
      typedef dtl::DBView< DBRep<T> > DBViewType;

      DBViewType view(tableName(), BCA<T>());
      typename DBViewType::insert_iterator  iter = view;

      // copy info of the T to the DBRep<T> class
      DBRep<T>    rec;
      toDatabaseRep (rec);

      // save this record
      *iter = rec;

      // Once we've reached here, the insert was successful.
      // Increment the version number.
      metaData().versionNr()++;
    }


    template<typename T>
    void TPersistentObject<T>::doUpdate() const
    {
      typedef dtl::DBView< DBRep<T>, DBRep<ObjectId> > DBViewType;

      DBViewType view(tableName(), BCA<T>(), 
                      "WHERE ObjId=(?)", BPA<ObjectId>());
      typename DBViewType::update_iterator iter = view;

      // copy info of the T to the DBRep<T> class
      DBRep<T>    rec;
      toDatabaseRep (rec);

      // setup the selection parameters
      iter.Params().itsOid = rec.getOid();

      // save this record
      *iter = rec;

      // Once we've reached here, the update was successful.
      // Increment the version number.
      metaData().versionNr()++;
    }


    template<typename T>
    void TPersistentObject<T>::doRetrieve(const ObjectId& oid, bool isOwnerOid)
    {
      std::string whereClause;
      if (isOwnerOid)
        whereClause = "WHERE Owner=(?)";
      else
        whereClause = "WHERE ObjId=(?)";

      typedef dtl::DBView< DBRep<T>, DBRep<ObjectId> > DBViewType;
      DBViewType view(tableName(), BCA<T>(), whereClause, BPA<ObjectId>());
      typename DBViewType::select_iterator iter = view.begin();

      iter.Params().itsOid = oid.get();

      // We should find a match! Otherwise there's some kind of logic error.
      AssertStr(iter != view.end(), "oid=" << oid.get()
                << ", isOwnerOid=" << (isOwnerOid ? "true" : "false") );  
      fromDatabaseRep(*iter);

    }

  } // namespace PL

} // namespace LOFAR

#endif
