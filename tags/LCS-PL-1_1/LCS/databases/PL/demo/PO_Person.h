//# This file was generated by genDBcode v2.8 on Tue Mar  9 16:39:41 CET 2004
//# with the command: genDBcode P Person.plmap.proto  
//# from the directory: /home/loose/LOFAR/LCS/databases/PL/demo
//#
//# EDITING THIS FILE MANUALLY IS AT YOUR OWN RISK
//# IT MIGHT BE OVERWRITTEN BY THE NEXT MAKE OF YOUR PROJECT
//#
//# only include this file once
#if !defined(PO_040309163941_PERSON_H)
#define PO_040309163941_PERSON_H

#include "Person.h"
#include <PL/DBRep.h>

namespace LOFAR {
  namespace PL {


    // The DBRep< Person > structure is a contigious representation of
    // all fields that should be stored to the database
    template <>
    struct DBRep< Person > {
      void bindCols(dtl::BoundIOs& cols);
      std::string      name;
      std::string      address;
      int      age;
      int      gender;
    };

  } // close namespace PL
}  // close namespace LOFAR

#include "PO_Person.tcc"  // Include template code

#endif
