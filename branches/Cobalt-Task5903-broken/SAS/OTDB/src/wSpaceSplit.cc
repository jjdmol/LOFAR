//#  wSpaceSplit.cc: Utility for splitting whitespace separated strings
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
#include <Common/StringUtil.h>
#include <OTDB/wSpaceSplit.h>

namespace LOFAR {
  namespace OTDB {

//
// wSpaceSplit
//
// Splits the given string into a vector of substring using white space
// as the seperator between the elements.
// When the max number of substrings is limited the 'remainder' of the input-
// string is returned as the last element.
//
// Syntax: <element> <whitespace>  <element> <whitespace>  <element> ...
//
// An element may be placed between single or double qoutes.
//
vector<string> wSpaceSplit(const string& targetStr, uint16	maxStrings)
{
	// make destroyable copy
	uint16			MaxLineLen = 1024;
	char			buffer[MaxLineLen];
	char*			target 	= &buffer[0];

	uint32	tLen = targetStr.copy(buffer, MaxLineLen, 0); 
	buffer[tLen] = '\0';

	// rtrim target
	uint32	lastPos = rtrim(target) - 1;

	// scan string and construct vector
	vector<string>		result;
	uint16				nrStrings = 0;
	uint32				idx = 0;
	while (idx <= lastPos && nrStrings < maxStrings) {
		// skip leading space.
		while(idx <= lastPos && (target[idx]==' ' || target[idx]=='\t')) {
			++idx;
		}
		uint32		start = idx;		// remember first char.

		// does argument start with single or double quote?
		if (target[idx] == '\'' || target[idx] == '"') {
			++idx;
			// find matching qoute
			while (idx <= lastPos && target[idx] != target[start]) {
				++idx;
			}
			if (target[idx] != target[start]) {	// reached EOL?
				THROW(Exception,
					formatString("%s: unmatched quote", target+start));
			}
			// start and idx now at qoutes of same type, remove both
			// except when we are NOT at end of line but ARE at last element
			if (++nrStrings <= maxStrings || !target[idx+1]) {
				target[start++] = '\0';
				target[idx++]   = '\0';
			}
			result.push_back(string(target+start));
			continue;
		}

		// no quotes around element, find next whitespace
		while(idx <= lastPos && (target[idx]!=' ' && target[idx]!='\t')) {
			++idx;
		}

		if (idx != start) {
			if (++nrStrings < maxStrings) {
				target[idx++]   = '\0';
			}
			result.push_back(target+start);
		}
	}

	return (result);
}

  } // namespace OTDB
} // namespace LOFAR
