//# This file was generated by genLCScode v1.0 on Tue Nov 11 17:20:45 CET 2003
//# with the command: genLCScode MyModule.map.proto MyModule.fun.proto P 
//# from the directory: /export/home/loose/temp
//#
//# EDITING THIS FILE MANUALLY IS AT YOUR OWN RISK
//# IT MIGHT BE OVERWRITTEN BY THE NEXT MAKE OF YOUR PROJECT
//#
//# only include this file once
#ifndef LOFAR_PL_TEST_C_H
#define LOFAR_PL_TEST_C_H

#include "C.h"
#include <PL/ObjectId.h>

namespace LOFAR {

  namespace PL {

    // The DBRep<C> structure is a compilation of the fields
    // of the C class and the persistency layer. It contains
    // all fields that should be stored to the database
    template<>
    struct DBRep<C> {
      ObjectId::oid_t  itsOid;
      ObjectId::oid_t  itsOwnerOid;
      unsigned int     itsVersionNr;
      blob             itsBlob;
      string           itsString;
    };

  } // close namespace PL

}  // close namespace LOFAR

#include "PO_C.tcc"  // Include template code

#endif
