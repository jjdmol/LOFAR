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

#include <PL/DBRepHolder.h>
#include <PL/DTLBase.h>
#include <PL/Exception.h>
#include <PL/TPersistentObject.h>
#include <PL/QueryObject.h>
#include <PL/Collection.h>
#include <Common/LofarLogger.h>
#include <dtl/DBView.h>
#include <dtl/select_iterator.h>
#include <sstream>

namespace LOFAR
{
  namespace PL
  {

    template<typename T> typename TPersistentObject<T>::attribmap_t
    TPersistentObject<T>::theirAttribMap;


    template<typename T>
    const typename TPersistentObject<T>::attribmap_t& 
    TPersistentObject<T>::attribMap() const
    {
      static bool initialized(false);
      if (!initialized)
      {
        initAttribMap();
        initialized = true;
      }
      return theirAttribMap; 
    }


    template<typename T>
    Collection< TPersistentObject<T> > 
    TPersistentObject<T>::retrieve(const QueryObject& query, int maxObjects)
    {
      typedef dtl::DBView< DBRepHolder<T> >  DBViewType;

      Collection< TPersistentObject<T> > ctpo;

      try {
        DBViewType view(tableName(), BCA<T>(), query.getSql());
        LOG_TRACE_VAR_STR (__PRETTY_FUNCTION__ << "\n  " << query.getSql());
        typename DBViewType::select_iterator iter = view.begin();

        for (int nr = 0; iter != view.end() && nr < maxObjects; ++iter, ++nr) {
          TPersistentObject<T> tpo;
          tpo.tableName (tableName());
          tpo.fromDBRep(*iter);          // Retrieve this record

          // If the object T is spread among several tables we must call
          // retrieve() in order to get the data from all the tables. If we
          // don't, we will miss the data for the "owned" POs. This really
          // isn't very efficient! However, currently there is no way to do
          // things better. We need a way to generate a better select query
          // for this.
          if (!tpo.ownedPOs().empty()) {
            tpo.retrieve();
          }
          ctpo.add(tpo);
        }

      }
      catch(dtl::DBException& e) {
        THROW (RetrieveError, "Retrieve failed.\n" << e.what());
      }
      return ctpo;
    }


    template<typename T>
    int 
    TPersistentObject<T>::retrieveInPlace(const QueryObject& query,
					  int maxObjects)
    {
      int nr=0;
      typedef dtl::DBView< DBRepHolder<T> >  DBViewType;
      try {
        DBViewType view(tableName(), BCA<T>(), query.getSql());
        LOG_TRACE_VAR_STR (__PRETTY_FUNCTION__ << "\n  " << query.getSql());
	// Only use the first record found.
	typename DBViewType::select_iterator iter = view.begin();
	if (iter != view.end()) {
	  fromDBRep(*iter);          // Retrieve this record
	  // Get the data from the nested PO-s.
	  if (!ownedPOs().empty()) {
	    retrieve();
	  }
	  // Get total nr of records found.
	  for (; iter!=view.end() && nr<maxObjects; ++iter) {
	    nr++;
	  }
        }
      }
      catch(dtl::DBException& e) {
        THROW (RetrieveError, "Retrieve failed.\n" << e.what());
      }
      return nr;
    }


    template<typename T>
    void TPersistentObject<T>::doErase() const
    {
      typedef dtl::DBView< DBRepHolder<ObjectId> > DBViewType;

      try {
        DBViewType view(tableName(), BCA<ObjectId>());
        typename DBViewType::delete_iterator iter = view;
        
        // setup the selection parameters
        DBRepHolder<ObjectId> rec;
        rec.rep().itsOid = metaData().oid()->get();
        *iter = rec;        // Delete this record
      }
      catch (dtl::DBException& e) {
        THROW (EraseError, "Erase failed.\n" << e.what());
      }
      
      // Once we've reached here, the update was successful.
      // Reset the metadata structure.
      metaData().reset();
    }


    template<typename T>
    void TPersistentObject<T>::doInsert() const
    {
      typedef dtl::DBView< DBRepHolder<T> > DBViewType;

      try {
        DBViewType view(tableName(), BCA<T>());
        typename DBViewType::insert_iterator  iter = view;
        
        // copy info of the T to the DBRepHolder<T> class
        DBRepHolder<T>    rec;
        toDBRep (rec);
        *iter = rec;        // Save this record
      }
      catch(dtl::DBException& e) {
        THROW (InsertError, "Insert failed.\n" << e.what());
      }

      // Once we've reached here, the insert was successful.
      // Increment the version number.
      metaData().versionNr()++;
    }


    template<typename T>
    void TPersistentObject<T>::doRetrieve(const ObjectId& oid, bool isOwnerOid)
    {
      std::ostringstream whereClause;
      if (isOwnerOid) {
        whereClause << "WHERE Owner=" << oid.get();
      }	else {
        whereClause << "WHERE ObjId=" << oid.get();
      }

      typedef dtl::DBView< DBRepHolder<T>, DBRepHolder<ObjectId> > DBViewType;

      try {
        DBViewType view(tableName(), BCA<T>(), whereClause.str());
        typename DBViewType::select_iterator iter = view.begin();

        // We should find a match! Otherwise, the database record was probably
        // deleted by another thread or process.
        if (iter == view.end()) {
          THROW (RetrieveError, 
                 "Retrieve failed. Matching record could not be found;\n "
                 "it may have been deleted by another thread or process");
        }
        fromDBRep(*iter);      // Retrieve this record
      }
      catch(dtl::DBException& e) {
        THROW (RetrieveError, "Retrieve failed.\n" << e.what());
      }

    }


    template<typename T>
    void TPersistentObject<T>::doUpdate() const
    {
      std::ostringstream whereClause;
      whereClause << "WHERE ObjId=" << metaData().oid()->get();

      typedef dtl::DBView< DBRepHolder<T>, DBRepHolder<ObjectId> > DBViewType;

      try {
        DBViewType view(tableName(), BCA<T>(), whereClause.str());
        typename DBViewType::update_iterator iter = view;
        
        // copy info of the T to the DBRepHolder<T> class
        DBRepHolder<T>    rec;
        toDBRep (rec);
        *iter = rec;      // Save this record
      }
      catch (dtl::DBException& e) {
        THROW (UpdateError, "Update failed.\n" << e.what());
      }

      // Once we've reached here, the update was successful.
      // Increment the version number.
      metaData().versionNr()++;
    }

    template<typename T>
    void TPersistentObject<T>::toDBRep(DBRepHolder<T>& dest) const
    {
      toDBRepMeta(dest.repMeta());
      toDBRep(dest.rep());
    }

    // Convert the data from DBRep format to our persistent object.
    template<typename T>
    void TPersistentObject<T>::fromDBRep(const DBRepHolder<T>& src)
    {
      fromDBRepMeta(src.repMeta());
      fromDBRep(src.rep());
    }

  } // namespace PL

} // namespace LOFAR

#endif
