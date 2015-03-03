//#  TBBReadCmd.cc: one_line_description
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <VHECR/TBBReadCmd.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace LOFAR {
  namespace VHECR {

using namespace boost::posix_time;

//
// TBBReadCmd(...)
//
TBBReadCmd::TBBReadCmd (uint32	rcuNr, 	uint32	time, 	uint32	sampleNr,
						uint32	prePages, 	uint32	postPages) :
	itsRcuNr	 (rcuNr),
	itsTime 	 (time),
	itsSampleNr  (sampleNr),
	itsPrePages	 (prePages),
	itsPostPages (postPages)
{}

// 
// TBBReadCmd()
//
TBBReadCmd::TBBReadCmd() :
	itsRcuNr	 (0),
	itsTime 	 (0),
	itsSampleNr  (0),
	itsPrePages	 (0),
	itsPostPages (0)
{}

//
// operator=
//
TBBReadCmd& TBBReadCmd::operator=(const TBBReadCmd& that)
{
	if (this != &that) { 
		this->itsRcuNr 	    = that.itsRcuNr; 
		this->itsTime 	    = that.itsTime; 
		this->itsSampleNr   = that.itsSampleNr; 
		this->itsPrePages   = that.itsPrePages; 
		this->itsPostPages  = that.itsPostPages;
	} 

	return (*this); 
}

//#
//# operator<<
//#
ostream& TBBReadCmd::print(ostream&	os) const
{
	os << "RCUnr      : " << itsRcuNr << endl;
	os << "Time       : " << to_simple_string(from_time_t(itsTime)) << endl;
	os << "SampleNr   : " << itsSampleNr << endl;
	os << "Pre pages  : " << itsPrePages << endl;
	os << "Post pages : " << itsPostPages << endl;
	return (os);
}


  } // namespace VHECR
} // namespace LOFAR
