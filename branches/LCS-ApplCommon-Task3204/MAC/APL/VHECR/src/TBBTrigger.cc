//#  TBBTrigger.cc: one_line_description
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

#include "VHECR/TBBTrigger.h" // path for online use

namespace LOFAR {
  namespace VHECR {

//
// TBBTrigger(...)
//
	  TBBTrigger::TBBTrigger (uint32	rcuNr, 		uint32	time, 		uint32	sampleNr, RTC::NsTimestamp nsTimestamp,
						uint32	sum, 	uint32	nrSamples,	uint32	peakValue,	uint32	missed) :
	itsRcuNr	(rcuNr),
	itsTime 	(time),
	itsSampleNr	(sampleNr),
	itsNsTimestamp(nsTimestamp),
	itsSum		(sum),
	itsNrSamples(nrSamples),
	itsPeakValue(peakValue),
	itsMissed	(missed)
{}

// 
// TBBTrigger()
//
TBBTrigger::TBBTrigger() :
	itsRcuNr	(0),
	itsTime 	(0),
	itsSampleNr	(0),
	itsNsTimestamp(0),
	itsSum		(0),
	itsNrSamples(0),
	itsPeakValue(0),
	itsMissed	(0) 
{}

//
// operator=
//
TBBTrigger& TBBTrigger::operator=(const TBBTrigger& that)
{
	if (this != &that) { 
		this->itsRcuNr 	   = that.itsRcuNr; 
		this->itsTime 	   = that.itsTime; 
		this->itsSampleNr  = that.itsSampleNr; 
		this->itsNsTimestamp = that.itsNsTimestamp;
		this->itsSum 	   = that.itsSum; 
		this->itsNrSamples = that.itsNrSamples; 
		this->itsPeakValue = that.itsPeakValue; 
		this->itsMissed	   = that.itsMissed; 
	} 

	return (*this); 
}

//#
//# operator<<
//#
ostream& TBBTrigger::print(ostream&	os) const
{
	os << "RCUnr      : " << itsRcuNr << endl;
		char	*timeStr = ctime((const time_t*)&itsTime);
	timeStr[strlen(timeStr)-1] = '\0';
	os << "Time       : " << timeStr << endl;
	os << "SampleNr   : " << itsSampleNr << endl;
	os << "Time sec   : " << itsNsTimestamp.sec() << endl;
	os << "Time nsec  : " << itsNsTimestamp.nsec() << endl;
	os << "Sum        : " << itsSum << endl;
	os << "Nr samples : " << itsNrSamples << endl;
	os << "Peakvalue  : " << itsPeakValue << endl;
	os << "Missed     : " << itsMissed << endl;

	return (os);
}


// @}
  } // namespace VHECR
} // namespace LOFAR

