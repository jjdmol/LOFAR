//#  ParCollRecord.cc: ParameterCollection record for DB storage
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

#include <lofar_config.h>
#include "ParCollRecord.h"

namespace LOFAR {
  namespace ACC {

ParCollRecord::ParCollRecord (const string&		aName,
							  const string&		aVersion,
							  const string&		aQualification,
							  const string&		aContents) :
	itsName(aName),
	itsVersionNr(aVersion),
	itsQualification(aQualification),
	itsCollection(aContents)
{
}


int32 ParCollRecord::getLine(string&		buffer, uint32*	offset)
{
	if (*offset >= itsCollection.size()) {		// reached end, reset
		buffer = "";
		*offset = itsCollection.size();
		return (0);
	}

	int32 eol 		= itsCollection.find_first_of ('\n', *offset);
	int32 resultLen = eol - *offset;
	buffer = itsCollection.substr(*offset, resultLen);
	*offset = eol+1;
	return (resultLen);
}



  } // namespace ACC
} // namespace LOFAR
