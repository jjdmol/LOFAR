//#  OTDBtypes.h: Size definitions of database identifier-fields.
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

#ifndef LOFAR_OTDB_OTDBTYPES_H
#define LOFAR_OTDB_OTDBTYPES_H

// \file
// Size definitions of database identifier-fields.

#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

typedef		int32		treeIDType;
//typedef		int64		nodeIDType;	TODO: long long not supported by pqxx
typedef		int32		nodeIDType;
typedef		int32		actionIDType;
typedef		int32		eventIDType;

typedef		int16		classifType;
typedef		int16		treeType;
typedef		int16		treeState;
typedef		int16		unitType;
typedef		int16		paramType;
typedef		int16		actionType;
typedef		int16		eventType;

// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
