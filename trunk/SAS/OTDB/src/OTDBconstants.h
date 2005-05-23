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
const int16 PTnode				= 0;		// not a parameter
const int16	PTbool				= 101;
const int16	PTint				= 102;
const int16	PTlong				= 103;
const int16	PTfloat				= 104;
const int16	PTdouble			= 105;
const int16	PTicomplex			= 106;
const int16	PTlcomplex			= 107;
const int16	PT_fcomplex			= 108;
const int16	PTdcomplex			= 109;
const int16	PTtext				= 110;
const int16	PTbin				= 111;


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
