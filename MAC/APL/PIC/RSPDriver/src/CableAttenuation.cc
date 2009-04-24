//#  CableAttenuation.cc: one line description
//#
//#  Copyright (C) 2009
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
#include "CableAttenuation.h"
#include <boost/algorithm/string.hpp>

namespace LOFAR {
  using namespace blitz;

CableAttenuation::~CableAttenuation()
{}

CableAttenuation::CableAttenuation(const string&	filename)
{
	#define MAX_RCU_MODE	7

	// open the file
	ConfigLocator	CL;
	string	theFile(CL.locate(filename));
	ASSERTSTR(!theFile.empty(), "File " << filename << " not found");
	LOG_DEBUG_STR("Reading attenuations from file: " << theFile);
	
	ifstream		caStream;
	caStream.open(theFile.c_str(), ifstream::in);
	ASSERTSTR(caStream, "Unable to open the file '" << theFile << "'");

	// parse the file
	string	line;
	int		prevRcuMode(-1);
	int		nrOfColumns(-1);
	getline(caStream, line);
	while(caStream) {
		LOG_TRACE_VAR_STR("rawline:>" << line << "<");
		if (line[0] != '#' && line[0] != ' ' && line[0] != '\0') {
			// syntax: rcumode <serie of cable-lengths
			vector<string>	column;
			boost::replace_all(line, "\t", " ");
			boost::replace_all(line, "  ", " ");
			boost::split(column, line, boost::is_any_of(" "));

			// First line (with rcumode 0) contains the cable lengths
			if (prevRcuMode == -1) {
				nrOfColumns = column.size();
				LOG_TRACE_STAT_STR("Found " << nrOfColumns << " columns on line 0");
				ASSERTSTR(nrOfColumns > 1, "Minimal 1 cable length expected");
				ASSERTSTR(strToInt(column[0]) == 0, "Table must begin with line for rcumode 0");

				// alloc storage and store cable lengths in seperate array.
				itsAtts.resize(MAX_RCU_MODE + 1, nrOfColumns-1);
				itsAtts = 0.0;
				itsCableLengths.resize(nrOfColumns-1);
				itsCableLengths = 0;
				for (int colNr = 1; colNr < nrOfColumns; colNr++) {
					itsCableLengths(colNr-1) = strToInt(column[colNr]);
				}
				LOG_DEBUG_STR("Cable lenghts: " << itsCableLengths);
			}
			else {
				// not the first line, do some sanity checks before storing the atts.
				int rcuMode = strToInt(column[0]);
				ASSERTSTR(rcuMode == prevRcuMode + 1, "Expected line with rcumode " << prevRcuMode + 1);
				ASSERTSTR(column.size() == nrOfColumns, "Expected " << nrOfColumns << " fields on line: " << line);
				ASSERTSTR(rcuMode <= MAX_RCU_MODE, 
							"RCUmode " << rcuMode << " not in range [0.." << MAX_RCU_MODE << "]");

				// copy values to internal array.
				for (int	colNr = 1; colNr < nrOfColumns; colNr++) {
					itsAtts(prevRcuMode + 1, colNr - 1) = strToFloat(column[colNr]);
				}
			}
			// update admin and go on
			prevRcuMode++;
		}
		getline(caStream, line);
	} // while
	caStream.close();

	ASSERTSTR(prevRcuMode == MAX_RCU_MODE, "Expected settings for all " << MAX_RCU_MODE << " rcumodes.");
	LOG_DEBUG_STR(itsAtts);
}

//
// isLegalLength(cableLength)
//
bool	CableAttenuation::isLegalLength (int	cableLength) const
{
	try {
		(void) cableLen2Index(cableLength);
		return (true);
	}
	catch (Exception&	e) {
	}

	return (false);
}

// 
// cableLen2Index(cableLen)
//
// Converts a cablelength into an index in the itsAtts array.
int CableAttenuation::cableLen2Index(int	cableLen) const
{
	for (int i = itsCableLengths.size() -1; i >= 0; i--) {
		if (itsCableLengths(i) == cableLen) {
			return (i);
		}
	}
	ASSERTSTR(false, "Cablelength " << cableLen << " is not a legal cablelength:" << itsCableLengths);
}

// 
// getAttenuation(cableLen, rcuMode)
//
// Returns the attenuation in dB for the given cable length and rcumode.
float	CableAttenuation::getAttenuation(int	cableLength, int	rcuMode) const
{
	ASSERTSTR(rcuMode >= 0 && rcuMode <= MAX_RCU_MODE, 
							"RCUmode " << rcuMode << " not in range [0.." << MAX_RCU_MODE << "]");
	return (itsAtts(rcuMode, cableLen2Index(cableLength)));
}

} // namespace LOFAR
