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
    void DBRep<B>::bindCols(BoundIOs& cols)
    {
      cols["ItsBool"]   == itsBool;
      cols["ItsShort"]  == itsShort;
      cols["ItsFloat"]  == itsFloat;
      cols["ItsString"] == itsString;
    }

    // toDBRep copies the fields of the B class to the DBRep<B> structure.
    void DBRep<B>::toDBRep(const B& src)
    {
      itsBool = src.itsBool;
      itsShort  = src.itsShort;
      itsFloat  = src.itsFloat;
      itsString  = src.itsString;
    }


    // fromDBRep copies the fields of the DBRep<B> structure to the B class.
    void DBRep<B>::fromDBRep(B& dest) const
    {
      dest.itsBool  = itsBool;
      dest.itsShort  = itsShort;
      dest.itsFloat  = itsFloat;
      dest.itsString  = itsString;
    }

    // Initialize the internals of TPersistentObject<B>
    template<>
    void TPersistentObject<B>::init()
    {
      // Set the correct database table name
      tableName("B");
    }

    // Initialize the attribute map for TPersistentObject<B>
    void TPersistentObject<B>::initAttribMap()
    {
      theirAttribMap["itsBool"]   = "ITSBOOL";
      theirAttribMap["itsShort"]  = "ITSSHORT";
      theirAttribMap["itsFloat"]  = "ITSFLOAT";
      theirAttribMap["itsString"] = "ITSSTRING";
    }

  } // close namespace PL

}  // close namespace LOFAR

