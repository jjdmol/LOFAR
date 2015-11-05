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
#include <Common/hexdump.h>
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

	// First parse the file to count the lines. 
	// (Incrementing 2 dimensional blitz-arrays doesn't work the it should.)
	string	line;
	int		nrLines(0);
	getline(caStream, line);
	while(caStream) {
		if (line[0] != '#' && line[0] != ' ' && line[0] != '\0') {
			nrLines++;
		}
		getline(caStream, line);
	} // while

	// Allocate the arrays.
	itsCableLengths.resize(nrLines);
	itsCableLengths = 0;
	itsAtts.resize(MAX_RCU_MODE + 1, nrLines);
	itsAtts = 0.0;
	
	// Finally parse the file
	int		nrOfColumns(-1);
	int		lineIndex  (0);
	caStream.clear();			// rewind file and start scan
	caStream.seekg(0);
	getline(caStream, line);
	while(caStream) {
		LOG_TRACE_VAR_STR("rawline:>" << line << "<");
		if (line[0] != '#' && line[0] != ' ' && line[0] != '\0') {
			// syntax: rcumode <serie of cable-lengths
			vector<string>	column;
			boost::replace_all(line, "\t", " ");
			size_t	lineLen;
			do {
				lineLen = line.size();
				boost::replace_all(line, "  ", " ");
			} while (lineLen != line.size());
			boost::split(column, line, boost::is_any_of(" "));
			ASSERTSTR(column.size() == 8, "Each line needs 8 columns(" << column.size() << "):" << line);

			// copy info to arrays
			itsCableLengths(lineIndex)=strToInt(column[0]);
			nrOfColumns = column.size();
			for (int colNr = 1; colNr < nrOfColumns; colNr++) {
				itsAtts(colNr, lineIndex) = strToFloat(column[colNr]);
//				cout << colNr << "," << lineIndex << " = " << strToFloat(column[colNr]) << endl;
			}
			lineIndex++;
		}
		getline(caStream, line);
	} // while
	caStream.close();

	LOG_DEBUG_STR("Cable lenghts: " << itsCableLengths);
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
