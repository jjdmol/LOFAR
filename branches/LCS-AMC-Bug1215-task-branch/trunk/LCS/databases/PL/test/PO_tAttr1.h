//# This file was generated by genDBcode v2.9 on Mon Mar 15 15:31:32 CET 2004
//# with the command: genDBcode P tAttr1.plmap  
//# from the directory: /home/gvd/sim/LOFAR/LCS/databases/PL/test
//#
//# EDITING THIS FILE MANUALLY IS AT YOUR OWN RISK
//# IT MIGHT BE OVERWRITTEN BY THE NEXT MAKE OF YOUR PROJECT
//#
//# only include this file once
#if !defined(PO_040315153132_TATTR1_H)
#define PO_040315153132_TATTR1_H

#include "tAttr1.h"
#include <PL/DBRep.h>

namespace LOFAR {
	namespace PL {


// The DBRep< A > structure is a contigious representation of
// all fields that should be stored to the database
template <>
struct DBRep< A > {
	void bindCols(dtl::BoundIOs& cols);
	int			s;
};


// The DBRep< B > structure is a contigious representation of
// all fields that should be stored to the database
template <>
struct DBRep< B > {
	void bindCols(dtl::BoundIOs& cols);
	int			s;
};


// The DBRep< C > structure is a contigious representation of
// all fields that should be stored to the database
template <>
struct DBRep< C > {
	void bindCols(dtl::BoundIOs& cols);
	int			s;
};


// The DBRep< D > structure is a contigious representation of
// all fields that should be stored to the database
template <>
struct DBRep< D > {
	void bindCols(dtl::BoundIOs& cols);
	int			s;
};


// The DBRep< E > structure is a contigious representation of
// all fields that should be stored to the database
template <>
struct DBRep< E > {
	void bindCols(dtl::BoundIOs& cols);
	int			s;
};


// The DBRep< F > structure is a contigious representation of
// all fields that should be stored to the database
template <>
struct DBRep< F > {
	void bindCols(dtl::BoundIOs& cols);
	int			s;
};


	} // close namespace PL
}	// close namespace LOFAR

#include "PO_tAttr1.tcc"	// Include template code

#endif
