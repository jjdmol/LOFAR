//#  TBBTrigger.h: Definition of a struct for storing a trigger description
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#ifndef VHECR_TBBTRIGGER_H
#define VHECR_TBBTRIGGER_H

// Definition of a struct for storing a trigger description

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarTypes.h>

#include <Common/NsTimestamp.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace VHECR {

// \addtogroup StationCU
// @{


// class_description
class TBBTrigger
{
public:
	// Most likely way to construct the trigger.
	TBBTrigger (uint32 rcuNr, uint32 time, uint32 sampleNr, NsTimestamp nsTimestamp,
					uint32 sum, uint32 nrSamples, uint32 peakValue, uint32 missed);

	// default construction
	TBBTrigger();

	// Destructor
	~TBBTrigger() {};

	TBBTrigger& operator=(const TBBTrigger& that);

	// --- Datamembers ---
	// Note that the members are public, we use it as a struct.
	uint32          itsNo;
	uint32          itsRcuNr;
	uint32          itsTime;
	uint32          itsSampleNr;
	NsTimestamp 	itsNsTimestamp;
	uint32          itsSum;
	uint32          itsNrSamples;
	uint32          itsPeakValue;
	uint32          itsMissed;

	//# print function for operator<<
	ostream&	print(ostream&	os) const;
};

//#
//# operator<<
//#
inline ostream& operator<< (ostream& os, const TBBTrigger& aTBBTrigger)
{	
	return (aTBBTrigger.print(os));
}


// @}
  } // namespace VHECR
} // namespace LOFAR

#endif
