//# This file was generated by genLCScode v1.0 on Tue Nov 11 17:20:45 CET 2003
//# with the command: genLCScode MyModule.map.proto MyModule.fun.proto P 
//# from the directory: /export/home/loose/temp
//#
//# EDITING THIS FILE MANUALLY IS AT YOUR OWN RISK
//# IT MIGHT BE OVERWRITTEN BY THE NEXT MAKE OF YOUR PROJECT
//#
#include "PO_B.h"
#include "LCS_base.h"
#include <PL/Collection.h>
#include <PL/DTLHelperClasses.h>
#include <PL/Query.h>
#include <PL/TPersistentObject.h>
#include <string>

using namespace dtl;

namespace LOFAR {

  namespace PL {

    // The BCA<B> structure 'binds' the database columns
    // to the members of the DBRep<B> class.
    template<>
    void BCA<B>::operator()(BoundIOs& cols, DataObj& rowbuf) {
	  cols["ObjID"]     == rowbuf.itsOid;
	  cols["Owner"]     == rowbuf.itsOwnerOid;
	  cols["VersionNr"] == rowbuf.itsVersionNr;
	  cols["ItsBool"]   == rowbuf.itsBool;
	  cols["ItsShort"]  == rowbuf.itsShort;
	  cols["ItsFloat"]  == rowbuf.itsFloat;
	  cols["ItsString"] == rowbuf.itsString;
	}

    // toDatabaseRep copies the fields of the persistency layer
    // and of the B class to the given DBRep<B> structure
    template<>
    void TPersistentObject<B>::toDatabaseRep(DBRep<B>& dest) const
    {
      // copy info of the B to the DBRep<B> class
      // First copy the meta data
      dest.itsOid   = metaData().oid()->get();
      dest.itsOwnerOid = metaData().ownerOid()->get();
      dest.itsVersionNr  = metaData().versionNr();

      // Finally copy the info from B
      dest.itsBool = itsObjectPtr->itsBool;
      dest.itsShort  = itsObjectPtr->itsShort;
      dest.itsFloat  = itsObjectPtr->itsFloat;
      dest.itsString  = itsObjectPtr->itsString;
    }


    // fromDatabaseRep copies the fields of the DBRep<B> structure
    // to the persistency layer and the B class.
    template<>
    void TPersistentObject<B>::fromDatabaseRep(const DBRep<B>& org)
    {
      // copy info of the B to the DBRep<B> class
      // First copy the PO part
      metaData().oid()->set(org.itsOid);
      metaData().ownerOid()->set(org.itsOwnerOid);
      metaData().versionNr() = org.itsVersionNr;

      // Finally copy the info from B
      itsObjectPtr->itsBool  = org.itsBool;
      itsObjectPtr->itsShort  = org.itsShort;
      itsObjectPtr->itsFloat  = org.itsFloat;
      itsObjectPtr->itsString  = org.itsString;
    }

    //
    // Initialize the internals of TPersistentObject<B>
    //
    template<>
    void TPersistentObject<B>::init()
    {
    }

    //
    // Routine for insert this B object in the database.
    //
    template<>
    void TPersistentObject<B>::doInsert() const 
    {
      typedef DBView< DBRep<B> > DBViewType;

      DBViewType  insView("B", BCA<B>());
      DBViewType::insert_iterator  insIter = insView;

      // copy info of the B to the DBRep<B> class
      DBRep<B>    rec;
      toDatabaseRep (rec);

      // save this record
      *insIter = rec;

      // Once we've reached here, the insert was successful.
      // Increment the version number.
      metaData().versionNr()++;

      // No subclasses

    }


    //
    // Routine for updating this B object in the database.
    //
    template<>
    void TPersistentObject<B>::doUpdate() const 
    {
      typedef DBView<DBRep<B>, DBRep<ObjectId> > DBViewType;
      DBViewType updView("B", BCA<B>(), "WHERE ObjId=(?)", BPA<ObjectId>());
      DBViewType::update_iterator updIter = updView;

      // copy info of the B to the DBRep<B> class
      DBRep<B>    rec;
      toDatabaseRep (rec);

      // setup the selection parameters
      updIter.Params().itsOid = rec.itsOid;

      // save this record
      *updIter = rec;

      // No subclasses

      // Once we've reached here, the update was successful.
      // Increment the version number.
      metaData().versionNr()++;
    }

    //
    // Routine for deleting this B object in the database.
    //
    template<>
    void TPersistentObject<B>::doErase() const 
    {
      typedef DBView<DBRep<ObjectId> > DBViewType;
      DBViewType delView("B", BCA<ObjectId>());
      DBViewType::delete_iterator delIter = delView;

      // setup the selection parameters
      DBRep<ObjectId>     rec;
      rec.itsOid = metaData().oid()->get();

      // delete this record
      *delIter = rec;

      // No subclasses

      // Once we've reached here, the update was successful.
      // Reset the meta data structure.
      metaData().reset();
    }

    //
    // Routine to retrieve this B object from the database.
    //
    template<>
    Collection<TPersistentObject<B> > 
    TPersistentObject<B>::doRetrieve(const Query&  query, int maxObjects) 
    {
      std::cout << "doRetrieve(const Query&, int)" << std::endl;
      typedef DBView< DBRep<B> >  DBViewType;

      // GML: This isnt' quite right yet. Have to figure out how to construct
      // a DBView using a valid user-defined SQL statement
      DBViewType selView("B", BCA<B>(), query.getSql());

      DBViewType::select_iterator  selIter = selView.begin();
      Collection<TPersistentObject<B> >   selResult;

      for (int nrRecs = 0; selIter != selView.end() && nrRecs < maxObjects; ++selIter, ++nrRecs) {
	TPersistentObject<B>    TPOA;
	TPOA.fromDatabaseRep(*selIter);
	// No subclasses
	selResult.add(TPOA);
      }
      // @@@ TO BE DEFINED @@@

      return (selResult);
    }

    template<>
    void TPersistentObject<B>::doRetrieve(const ObjectId& aOid, 
                                          bool isOwnerOid)
    {
      std::string whereClause;
      if (isOwnerOid) {
        whereClause = "WHERE Owner=(?)";
      }
      else {
        whereClause = "WHERE ObjId=(?)";
      }

      typedef DBView< DBRep<B>, DBRep<ObjectId> > DBViewType;
      DBViewType selView("B", BCA<B>(), whereClause, BPA<ObjectId>());
      DBViewType::select_iterator selIter = selView.begin();

      selIter.Params().itsOid = aOid.get();

      // Should we throw an exception if there are no matching records?
      // Let's do it for the time being; that's easier for debugging.
      if (selIter != selView.end()) { 
	fromDatabaseRep(*selIter);
      }
      else {
	THROW(PLException,"No matching records found!");
      }
    }

    template class TPersistentObject<B>;

  } // close namespace PL

}  // close namespace LOFAR

