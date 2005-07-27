//#  OTDBconstants.h: Collection of constants also available in the DB.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_OTDB_OTDBCONSTANTS_H
#define LOFAR_OTDB_OTDBCONSTANTS_H

// \file OTDBconstants.h
// Collection of constants also available in the DB.

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

// Enumeration of tree classifications. Used in VIC and PIC trees.
const int16	TCall				= 0;
const int16	TCexperimental		= 1;
const int16	TCoperational		= 2;
const int16	TCobsolete			= 3;


// Enumeration of parameter types. Used in VIC and PIC trees.
//const string PTbool		("bool");
//const string PTint		("int");
//const string PTuint		("uint");
//const string PTlong		("long");
//const string PTfloat	("flt");
//const string PTdouble	("dbl");
//const string PTicomplex	("icpx");
//const string PTlcomplex	("lcpx");
//const string PTfcomplex	("fcpx");
//const string PTdcomplex	("dcpx");
//const string PTtext		("text");
//const string PTbin		("bin");


// Enumeration of tree types.
const int16	TThardware			= 10;
const int16	TTtemplate			= 20;
const int16	TTconfigure			= 30;
const int16	TTschedule			= 40;
const int16	TTqueued			= 50;
const int16	TTactive			= 60;
const int16	TTfinished			= 70;
const int16	TTobsolete			= 80;


// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
