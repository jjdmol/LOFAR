//# This file was generated by genDBcode v2.9 on Wed Mar  2 10:49:00 CET 2005
//# with the command: genDBcode P C.plmap  
//# from the directory: /home/loose/LOFAR/LCS/databases/PL/test
//#
//# EDITING THIS FILE MANUALLY IS AT YOUR OWN RISK
//# IT MIGHT BE OVERWRITTEN BY THE NEXT MAKE OF YOUR PROJECT
//#
//# only include this file once
#if !defined(PO_050302104900_B_H)
#define PO_050302104900_B_H

#include "B.h"
#include <PL/DBRep.h>

namespace LOFAR {
	namespace PL {


// The DBRep< B > structure is a contigious representation of
// all fields that should be stored to the database
template <>
struct DBRep< B > {
	void bindCols(dtl::BoundIOs& cols);
	bool			itsBool;
	short			itsShort;
	float			itsFloat;
	std::string			itsString;
};


	} // close namespace PL
}	// close namespace LOFAR

#include "PO_B.tcc"	// Include template code

#endif
