//# This file was generated by genDBcode v2.9 on Wed Mar  2 10:49:00 CET 2005
//# with the command: genDBcode P C.plmap  
//# from the directory: /home/loose/LOFAR/LCS/databases/PL/test
//#
//# EDITING THIS FILE MANUALLY IS AT YOUR OWN RISK
//# IT MIGHT BE OVERWRITTEN BY THE NEXT MAKE OF YOUR PROJECT
//#
#include "PO_B.h"
#include "PO_A.h"
#include <PL/TPersistentObject.h>

namespace LOFAR {
	namespace PL {

// bindCols() 'binds' the database columns represented by a cols
// to the members of the DBRep< A > class.
void DBRep< A >::bindCols(dtl::BoundIOs& cols) {
	cols["ITSINT"]	== itsInt;
	cols["ITSDOUBLE"]	== itsDouble;
	cols["ITSSTRING"]	== itsString;
	cols["ITSCOMPLEX_REAL"]	== itsComplex_real;
	cols["ITSCOMPLEX_IMAG"]	== itsComplex_imag;
}


// toDBRep copies the fields of the A class to the
// DBRep< A > structure
void TPersistentObject< A >::toDBRep(DBRep< A >& dest) const
{
	dest.itsInt	= data().itsInt;
	dest.itsDouble	= data().itsDouble;
	dest.itsString	= data().itsString;
	dest.itsComplex_real	= real(data().itsComplex);
	dest.itsComplex_imag	= imag(data().itsComplex);
}


// fromDBRep copies the fields of the DBRep< A > structure
// to the A class.
void TPersistentObject< A >::fromDBRep(const DBRep< A >& src)
{
	data().itsInt	= src.itsInt;
	data().itsDouble	= src.itsDouble;
	data().itsString	= src.itsString;
        data().itsComplex	= makedcomplex(src.itsComplex_real,
					       src.itsComplex_imag);
}


// Initialize the internals of TPersistentObject< A >
void TPersistentObject< A >::init()
{
	{
	// create new TPersistentObject for Class B
	Pointer p(new TPersistentObject< B >(data().itsB));
	// associate B's owner object-id with A's objectid
	p->metaData().ownerOid() = metaData().oid();
	// add newly created TPersistentObject to container of ownedPOs.
	ownedPOs().push_back(p);
	}
	// set the correct database table name
	tableName("A");
}


// Initialize the attribute map for TPersistenObject< A >
template<>
void TPersistentObject< A >::initAttribMap()
{
	theirAttribMap["itsInt"]	= "ITSINT";
	theirAttribMap["itsDouble"]	= "ITSDOUBLE";
	theirAttribMap["itsString"]	= "ITSSTRING";
	theirAttribMap["itsComplex_real"]	= "ITSCOMPLEX_REAL";
	theirAttribMap["itsComplex_imag"]	= "ITSCOMPLEX_IMAG";
	theirAttribMap["itsB"]	=
		"@" + string(typeid(TPersistentObject< B >).name());
}


	} // close namespace PL
}	// close namespace LOFAR

