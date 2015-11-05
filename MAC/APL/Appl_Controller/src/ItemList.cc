//#  ItemList.cc: one line description
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include "ItemList.h"

namespace LOFAR {
  namespace ACC {

ItemList::ItemList(const ParameterSet&	aPS,
				   const string&		prefix)
{
	int32	nrProcs = aPS.getInt32(prefix+"[0].count");
	string	procName;
	for (int32 p = 1; p <= nrProcs; p++) {
		procName = aPS.getString(formatString("%s[%d].ID", prefix.c_str(), p));
		int32	nrOfProcs = indexValue(procName, "()");
		if (nrOfProcs == 0) {
			push_back(procName);			// add to collection
			continue;
		}

		rtrim(procName, "(0123456789)");
		for (int32 idx = 1; idx <= nrOfProcs; ++idx) {
			push_back(formatString("%s[%d]", procName.c_str(), idx));
		}
	}
}

//ItemList::~ItemList()
//{}

bool ItemList::isInList(const string&	aProcName) const
{
	const_iterator		iter = begin();

	while (iter != end()) {
		if (*iter == aProcName) {
			return (true);
		}
		++iter;
	}

	return (false);
}


  } // namespace ACC
} // namespace LOFAR
