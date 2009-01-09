//#  RCUCables.cc: one line description
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
#include <APL/RTCCommon/RCUCables.h>
#include <boost/algorithm/string.hpp>

namespace LOFAR {
  namespace RTC {

  using namespace blitz;

#define MAX2(a,b) ((a) > (b) ? (a) : (b))

//
// Constructor
//
RCUCables::RCUCables(const string&	attFilename, const string&	delayFilename) :
	itsCableAtts(new CableAttenuation(attFilename))
{
	#define EXPECTED_NR_COLUMNS	7

	// open the file
	ConfigLocator	CL;
	string	theFile(CL.locate(delayFilename));
	ASSERTSTR(!theFile.empty(), "File " << delayFilename << " not found");
	LOG_DEBUG_STR("Reading cable-delays from file: " << theFile);
	
	ifstream		caStream;
	caStream.open(theFile.c_str(), ifstream::in);
	ASSERTSTR(caStream, "Unable to open the file '" << theFile << "'");

	// clear our arrays before starting the parse
	itsCableLengths.resize(MAX_RCUS, MAX_RCU_MODE);
	itsCableDelays.resize(MAX_RCUS, MAX_RCU_MODE);
	itsCableLengths = 0;
	itsCableDelays  = 0.0;

	// parse the file
	string	line;
	int		prevRcuNr(-1);
	getline(caStream, line);
	while(caStream) {
		LOG_TRACE_VAR_STR("rawline:>" << line << "<");
		if (line[0] != '#' && line[0] != ' ' && line[0] != '\0') {
			// syntax: rcuNr LBLlen LBLdelay LBHlen LBHdelay HBAlen HBAdelay
			int		rcuNr, LBLlen, LBHlen, HBAlen;
			float	LBLdelay, LBHdelay, HBAdelay;
			int		nrArgs = sscanf(line.c_str(), "%d %d %f %d %f %d %f", 
									&rcuNr, &LBLlen, &LBLdelay, &LBHlen, &LBHdelay, &HBAlen, &HBAdelay);

			// Do some sanity checks before continuing
			ASSERTSTR(rcuNr == prevRcuNr + 1, "Expected line with rcuNr " << prevRcuNr + 1);
			ASSERTSTR(rcuNr >= 0 && rcuNr < MAX_RCUS, "RCUNumber " << rcuNr << " not in range [0.." << MAX_RCUS-1 << "]");
			ASSERTSTR(nrArgs == EXPECTED_NR_COLUMNS, "Expected " << EXPECTED_NR_COLUMNS << " fields on line: " << line);

			ASSERTSTR(itsCableAtts->isLegalLength(LBLlen), "LBL cablelength " << LBLlen << " is not allowed");
			ASSERTSTR(itsCableAtts->isLegalLength(LBHlen), "LBH cablelength " << LBHlen << " is not allowed");
			ASSERTSTR(itsCableAtts->isLegalLength(HBAlen), "HBA cablelength " << HBAlen << " is not allowed");

			// copy values to internal arrays.
			itsCableLengths(rcuNr, 0) = LBLlen;
			itsCableLengths(rcuNr, 1) = LBHlen;
			itsCableLengths(rcuNr, 2) = HBAlen;
			itsCableDelays (rcuNr, 0) = LBLdelay;
			itsCableDelays (rcuNr, 1) = LBHdelay;
			itsCableDelays (rcuNr, 2) = HBAdelay;

			// keep track of longest cable in LBA and HBA group.
			itsLargestLBAlen   = MAX2(itsLargestLBAlen  , MAX2(LBLlen, LBHlen));
			itsLargestHBAlen   = MAX2(itsLargestHBAlen  , HBAlen);
			itsLargestLBAdelay = MAX2(itsLargestLBAdelay, MAX2(LBLdelay, LBHdelay));
			itsLargestHBAdelay = MAX2(itsLargestHBAdelay, HBAdelay);

			// update admin and go on
			prevRcuNr++;
		}
		getline(caStream, line);
	} // while
	caStream.close();
	ASSERTSTR(prevRcuNr != -1, "File " << delayFilename << " does not contain valid information");

	LOG_DEBUG_STR("Found cable specs for " << prevRcuNr << " RCUs");
	LOG_DEBUG_STR("Longest LBA cable is " << itsLargestLBAlen << "m");
	LOG_DEBUG_STR("Longest HBA cable is " << itsLargestHBAlen << "m");
	LOG_TRACE_STAT_STR(itsCableDelays);
}

RCUCables::~RCUCables()
{ }

// Returns the attenuation in dB for the given rcu when operation in the given rcumode.
float	RCUCables::getAtt  (int	rcuNr, int	rcuMode) const
{
	ASSERTSTR(rcuNr   >= 0 && rcuNr   <  MAX_RCUS,     "RCUNumber " << rcuNr   << " not in range [0.." << MAX_RCUS-1 << "]");
	ASSERTSTR(rcuMode >= 0 && rcuMode <= MAX_RCU_MODE, "RCUMode "   << rcuMode << " not in range [0.." << MAX_RCU_MODE << "]");

	switch (rcuMode) {
		case 0:	return (0.0);

		case 1:
		case 2: return (itsCableAtts->getAttenuation(itsCableLengths(rcuNr, 0), rcuMode));

		case 3:
		case 4: return (itsCableAtts->getAttenuation(itsCableLengths(rcuNr, 1), rcuMode));

		case 5:
		case 6:
		case 7: return (itsCableAtts->getAttenuation(itsCableLengths(rcuNr, 2), rcuMode));
	}
}

// Returns the delay in ns for the given rcu when operation in the given rcumode.
float	RCUCables::getDelay(int	rcuNr, int	rcuMode) const
{
	ASSERTSTR(rcuNr   >= 0 && rcuNr   <  MAX_RCUS,     "RCUNumber " << rcuNr   << " not in range [0.." << MAX_RCUS-1 << "]");
	ASSERTSTR(rcuMode >= 0 && rcuMode <= MAX_RCU_MODE, "RCUMode "   << rcuMode << " not in range [0.." << MAX_RCU_MODE << "]");

	switch (rcuMode) {
		case 0:	return (0.0);

		case 1:
		case 2: return (itsCableDelays(rcuNr, 0));

		case 3:
		case 4: return (itsCableDelays(rcuNr, 1));

		case 5:
		case 6:
		case 7: return (itsCableDelays(rcuNr, 2));
	}
}

// Returns the largest attenuation in dB when operation in the given rcumode.
float	RCUCables::getLargestAtt  (int	rcuMode) const
{
	return (itsCableAtts->getAttenuation((rcuMode < 5) ? itsLargestLBAlen : itsLargestHBAlen, rcuMode));
}

// Returns the largest delay in ns when operation in the given rcumode.
float	RCUCables::getLargestDelay(int	rcuMode) const
{
	return (rcuMode < 5 ? (rcuMode == 0) ? 0.0 : itsLargestLBAdelay : itsLargestHBAdelay);
}

  } // namespace RTC
} // namespace LOFAR
