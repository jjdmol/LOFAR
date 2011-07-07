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

// \file
// Collection of constants also available in the DB.

#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

// Enumeration of tree classifications. Used in VIC and PIC trees.
const int16	TCall				= 0;
const int16	TCdevelopment		= 1;
const int16	TCexperimental		= 2;
const int16	TCoperational		= 3;

// Enumeration of tree types.
const int16	TThardware			= 10;
const int16	TTtemplate			= 20;
const int16	TTVHtree			= 30;

// Enumeration of the tree states
const int16	TSidle				=    0;
const int16	TSbeingSpec			=  100;
const int16	TSspecified			=  200;
const int16	TSapproved			=  300;
const int16	TSscheduled			=  400;
const int16	TSqueued			=  500;
const int16	TSactive			=  600;
const int16	TSfinished			= 1000;
const int16	TSaborted			= 1100;
const int16	TSobsolete			= 1200;


// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
