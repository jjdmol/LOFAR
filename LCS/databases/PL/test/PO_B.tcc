//# This file was generated by genLCScode v1.0 on Tue Nov 11 17:20:45 CET 2003
//# with the command: genLCScode MyModule.map.proto MyModule.fun.proto P 
//# from the directory: /export/home/loose/temp
//#
//# EDITING THIS FILE MANUALLY IS AT YOUR OWN RISK
//# IT MIGHT BE OVERWRITTEN BY THE NEXT MAKE OF YOUR PROJECT
//#
#include "PO_B.h"
#include <PL/TPersistentObject.h>

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
      // Set the correct database table name
      tableName("B");
    }

  } // close namespace PL

}  // close namespace LOFAR

