//#  Event.cc: QAD implementation of the events between the AC and the PC
//#
//#  Copyright (C) 2002-2003
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

#include <ACC/Event.h>
#include <ACC/ParameterSet.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

//#-------------------------- creation and destroy ---------------------------
//#
//# Default constructor
//#
Event::Event()
{}

//#
//# Construction by reading a parameter file.
//#
Event::Event(const int16	theType,
			 const uint32	theTime,
			 const string	theInfo) :
	itsType(theType),
	itsTime(theTime),
	itsInfo(theInfo)
{ }

//#
//# Copying is allowed.
//#
Event::Event(const Event& that) :
	itsType(that.itsType),
	itsTime(that.itsTime),
	itsInfo(that.itsInfo)
{ }

//#
//# operator= copying
//#
Event& Event::operator=(const Event& that)
{
	if (this != &that) {
		itsType = that.itsType;
		itsTime = that.itsTime;
		itsInfo = that.itsInfo;
	}
	return (*this);
}

//#
//#	Destructor
//#
Event::~Event()
{}

//#
//# Construct an event from a parameter string
//#
Event::Event(const string		paramString)
{
	ParameterSet	theParams;
	theParams.adoptBuffer(paramString);

	itsType = theParams.getInt("event.type");
	itsTime = theParams.getInt("event.time");
	itsInfo = theParams.getString("event.info");

}
} // namespace LOFAR
