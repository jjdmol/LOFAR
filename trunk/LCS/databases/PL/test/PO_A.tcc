//# This file was generated by genLCScode v1.0 on Tue Nov 11 17:20:45 CET 2003
//# with the command: genLCScode MyModule.map.proto MyModule.fun.proto P 
//# from the directory: /export/home/loose/temp
//#
//# EDITING THIS FILE MANUALLY IS AT YOUR OWN RISK
//# IT MIGHT BE OVERWRITTEN BY THE NEXT MAKE OF YOUR PROJECT
//#
#include "PO_A.h"
#include "PO_B.h"
#include <PL/TPersistentObject.h>

using namespace dtl;

namespace LOFAR {

  namespace PL {

    // The BCA<A> structure 'binds' the database columns
    // to the members of the DBRep<A> class.
    void DBRep<A>::bindCols(BoundIOs& cols)
    {
      cols["ITSINT"]  == itsInt;
      cols["ITSDOUBLE"]  == itsDouble;
      cols["ITSSTRING"]  == itsString;
    }

    // toDBRep copies the fields of the A class to the DBRep<A> structure.
    void DBRep<A>::toDBRep(const A& src)
    {
      itsInt  = src.itsInt;
      itsDouble  = src.itsDouble;
      itsString  = src.itsString;
    }


    // fromDBRep copies the fields of the DBRep<A> structure to the A class.
    void DBRep<A>::fromDBRep(A& dest) const
    {
      dest.itsInt  = itsInt;
      dest.itsDouble  = itsDouble;
      dest.itsString  = itsString;
    }

    // Initialize the internals of TPersistentObject<A>
    void TPersistentObject<A>::init()
    {
      // create new TPersistentObject for B.
      Pointer p(new TPersistentObject<B>(itsObjectPtr->itsB));
      // associate B's owner object-id with the A's object-id
      p->metaData().ownerOid() = metaData().oid();
      // add newly created TPersistentObject to container of ownedPOs.
      ownedPOs().push_back(p);
      // Set the correct database table name
      tableName("A");
     }

    // Initialize the attribute map for TPersistentObject<A>
    template<>
    void TPersistentObject<A>::initAttribMap()
    {
      theirAttribMap["itsInt"]     = "ITSINT";
      theirAttribMap["itsDouble"]  = "ITSDOUBLE";
      theirAttribMap["itsString"]  =  "ITSSTRING";
      theirAttribMap["itsComplex"] = "ITSCOMPLEX";
      theirAttribMap["itsB"]       =
        "@" + string(typeid(TPersistentObject<B>).name());
    }

   } // close namespace PL

}  // close namespace LOFAR

