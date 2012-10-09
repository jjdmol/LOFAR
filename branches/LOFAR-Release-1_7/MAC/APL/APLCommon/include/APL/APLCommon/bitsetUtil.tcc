//#  bitsetUtil.h: Utility functions for bitsets
//#
//#  Copyright (C) 2008
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

#ifndef _APL_BITSETUTIL_TCC
#define _APL_BITSETUTIL_TCC

//# Common Includes
#include <Common/LofarTypes.h>
#include <Common/StringUtil.h>
#include <Common/lofar_bitset.h>
#include <Common/lofar_string.h>

namespace LOFAR { 
  namespace APLCommon {

// bitset2compactedArrayString(bitset)
template <typename	T>
string bitset2CompactedArrayString(const T&	theBits)
{
	if (!theBits.count()) {
		return ("[]");
	}

	string	result	 ("[");
	uint32	firstElem(0);
	bool	emptyset (true);
	uint32	nrBits = theBits.size();
	for (uint32 b = 1; b < nrBits; b++) {
		if (theBits.test(b)) {			// bit set?
			if (!theBits.test(b-1)) { 	// and previous bit was clear ?
				firstElem = b;			// remember this bit
			}
		}
		else {							// bit is clear
			if (theBits.test(b-1)) {  	// and the previous was set?
				if ((b-1) > firstElem) {	 // range?
					result += formatString("%s%d..%d", emptyset ? "" : ",", firstElem, b-1);
				}
				else {	// only one element
					result += formatString("%s%d", emptyset ? "" : ",", b-1);
				}
				emptyset = false;
			}
		}	
	}
	if (theBits.test(nrBits-1)) {		// add last part if last bit is set
		if ((nrBits-1) > firstElem) {	 // range?
			result += formatString("%s%d..%d", emptyset ? "" : ",", firstElem, nrBits-1);
		}
		else {	// only one element
			result += formatString("%s%d", emptyset ? "" : ",", nrBits-1);
		}
	}
		
	return (result+"]");
}

  };// namespace APLCommon
};// namespace LOFAR
#endif
