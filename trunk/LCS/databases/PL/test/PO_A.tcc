//# This file was generated by genLCScode v1.0 on Tue Nov 11 17:20:45 CET 2003
//# with the command: genLCScode MyModule.map.proto MyModule.fun.proto P 
//# from the directory: /export/home/loose/temp
//#
//# EDITING THIS FILE MANUALLY IS AT YOUR OWN RISK
//# IT MIGHT BE OVERWRITTEN BY THE NEXT MAKE OF YOUR PROJECT
//#
#include "PO_A.h"
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

    // The BCA<A> structure 'binds' the database columns
    // to the members of the DBRep<A> class.
    template<>
    void BCA<A>::operator()(BoundIOs& cols, DataObj& rowbuf) {
	  cols["ObjID"]    == rowbuf.itsOid;
	  cols["Owner"]  == rowbuf.itsOwnerOid;
	  cols["VersionNr"]  == rowbuf.itsVersionNr;
	  cols["ITSINT"]  == rowbuf.itsInt;
	  cols["ITSDOUBLE"]  == rowbuf.itsDouble;
	  cols["ITSSTRING"]  == rowbuf.itsString;
	}

    // toDatabaseRep copies the fields of the persistency layer
    // and of the A class to the given DBRep<A> structure
    template<>
    void TPersistentObject<A>::toDatabaseRep(DBRep<A>& dest) const
    {
      // copy info of the A to the DBRep<A> class
      // First copy the meta data
      dest.itsOid   = metaData().oid()->get();
      dest.itsOwnerOid = metaData().ownerOid()->get();
      dest.itsVersionNr  = metaData().versionNr();

      // Finally copy the info from A
      dest.itsInt  = itsObjectPtr->itsInt;
      dest.itsDouble  = itsObjectPtr->itsDouble;
      dest.itsString  = itsObjectPtr->itsString;
    }


    // fromDatabaseRep copies the fields of the DBRep<A> structure
    // to the persistency layer and the A class.
    template<>
    void TPersistentObject<A>::fromDatabaseRep(const DBRep<A>& org)
    {
      // copy info of the A to the DBRep<A> class
      // First copy the PO part
      metaData().oid()->set(org.itsOid);
      metaData().ownerOid()->set(org.itsOwnerOid);
      metaData().versionNr() = org.itsVersionNr;

      // Finally copy the info from A
      itsObjectPtr->itsInt  = org.itsInt;
      itsObjectPtr->itsDouble  = org.itsDouble;
      itsObjectPtr->itsString  = org.itsString;
    }

    //
    // Initialize the internals of TPersistentObject<A>
    //
    template<>
    void TPersistentObject<A>::init()
    {
      // create new TPersistentObject for B.
      Pointer p(new TPersistentObject<B>(itsObjectPtr->itsB));
      // associate B's owner object-id with the A's object-id
      p->metaData().ownerOid() = metaData().oid();
      // add newly created TPersistentObject to container of ownedPOs.
      ownedPOs().push_back(p);
    }

    //
    // Routine for insert this A object in the database.
    //
    template<>
    void TPersistentObject<A>::doInsert() const 
    {
      typedef DBView< DBRep<A> > DBViewType;

      DBViewType  insView("A", BCA<A>());
      DBViewType::insert_iterator  insIter = insView;

      // copy info of the A to the DBRep<A> class
      DBRep<A>    rec;
      toDatabaseRep (rec);

      // save this record
      *insIter = rec;

      // Once we've reached here, the insert was successful.
      // Increment the version number.
      metaData().versionNr()++;

      // No subclasses

    }


    //
    // Routine for updating this A object in the database.
    //
    template<>
    void TPersistentObject<A>::doUpdate() const 
    {
      typedef DBView<DBRep<A>, DBRep<ObjectId> > DBViewType;
      DBViewType updView("A", BCA<A>(), "WHERE ObjId=(?)", BPA<ObjectId>());
      DBViewType::update_iterator updIter = updView;

      // copy info of the A to the DBRep<A> class
      DBRep<A>    rec;
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
    // Routine for deleting this A object in the database.
    //
    template<>
    void TPersistentObject<A>::doErase() const 
    {
      typedef DBView<DBRep<ObjectId> > DBViewType;
      DBViewType delView("A", BCA<ObjectId>());
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
    // Routine to retrieve this A object from the database.
    //
    template<>
    Collection<TPersistentObject<A> > 
    TPersistentObject<A>::doRetrieve(const Query&  query, int maxObjects) 
    {
      std::cout << "doRetrieve(const Query&, int)" << std::endl;
      typedef DBView< DBRep<A> >  DBViewType;

      // GML: This isnt' quite right yet. Have to figure out how to construct
      // a DBView using a valid user-defined SQL statement
      DBViewType selView("A", BCA<A>(), query.getSql());

      DBViewType::select_iterator  selIter = selView.begin();
      Collection<TPersistentObject<A> >   selResult;

      for (int nrRecs = 0; selIter != selView.end() && nrRecs < maxObjects; ++selIter, ++nrRecs) {
	TPersistentObject<A>    TPOA;
	TPOA.fromDatabaseRep(*selIter);
	// No subclasses
	selResult.add(TPOA);
      }
      // @@@ TO BE DEFINED @@@

      return (selResult);
    }

    template<>
    void TPersistentObject<A>::doRetrieve(const ObjectId& aOid,
                                          bool isOwnerOid)
    {
      std::string whereClause;
      if (isOwnerOid) {
        whereClause = "WHERE Owner=(?)";
      }
      else {
        whereClause = "WHERE ObjId=(?)";
      }

      typedef DBView< DBRep<A>, DBRep<ObjectId> > DBViewType;
      DBViewType selView("A", BCA<A>(), whereClause, BPA<ObjectId>());
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

    template class TPersistentObject<A>;

  } // close namespace PL

}  // close namespace LOFAR

