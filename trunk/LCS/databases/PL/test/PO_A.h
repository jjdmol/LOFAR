//# This file was generated by genLCScode v1.0 on Tue Nov 11 17:20:45 CET 2003
//# with the command: genLCScode MyModule.map.proto MyModule.fun.proto P 
//# from the directory: /export/home/loose/temp
//#
//# EDITING THIS FILE MANUALLY IS AT YOUR OWN RISK
//# IT MIGHT BE OVERWRITTEN BY THE NEXT MAKE OF YOUR PROJECT
//#
//# only include this file once
#ifndef LOFAR_PL_TEST_A_H
#define LOFAR_PL_TEST_A_H

#include "A.h"
#include <PL/DBRep.h>

namespace LOFAR {

  namespace PL {

    // The DBRep<A> structure is a contiguous representation of all the fields
    // of the A class that should be stored in the database.
    template<>
    struct DBRep<A>
    {
      void bindCols(dtl::BoundIOs& cols);
      int              itsInt;
      double           itsDouble;
      std::string      itsString;
    };

  } // close namespace PL

}  // close namespace LOFAR

#include "PO_A.tcc"  // Include template code

#endif
