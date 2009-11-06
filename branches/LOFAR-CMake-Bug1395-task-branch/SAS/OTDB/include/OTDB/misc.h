//#  misc.h: Miscellaneous (global) functions.
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

#ifndef OTDB_MISC_H
#define OTDB_MISC_H

// \file
// Collection of global functions.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

uint32	VersionNr(const string&		VersString);
string	VersionNr(int32		VersNumber);
bool	isReference(const string&	limitsContents);
uint32	getVersionNrFromName(const string&	aName);
string	cleanNodeName(const string& aName);

// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
