//#  DH_ApplControl.cc: Implements the Application Controller command protocol.
//#
//#  Copyright (C) 2004
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
//#	 Abstract:
//#	 This class implements the command protocol between an application manager
//#	 (MAC for instance) and an Application Controller (=ACC package).
//#	 The AM has the client role, the AC the server role.
//#
//#  $Id$

#include <lofar_config.h>

//# Includes
#include <ACC/DH_ApplControl.h>


namespace LOFAR {
  namespace ACC {

// Constructor
DH_ApplControl::DH_ApplControl() :
	DataHolder ("", "DH_ApplControl"),
	itsVersionNumber(0),
	itsCommand(0),
	itsScheduleTime(0),
	itsResult(0),
	itsServerIP(0),
	itsServerPort(0)
{
	setExtraBlob ("Extra", 1);
}


// Destructor
DH_ApplControl::~DH_ApplControl() 
{ }

// Copying is allowed.
DH_ApplControl::DH_ApplControl(const DH_ApplControl& that) :
	DataHolder(that),
	itsVersionNumber(that.itsVersionNumber),
	itsCommand(that.itsCommand),
	itsScheduleTime(that.itsScheduleTime),
	itsResult(that.itsResult),
	itsServerIP(that.itsServerIP),
	itsServerPort(that.itsServerPort)
{
//TODO
}

DH_ApplControl*		DH_ApplControl::clone() const
{
	return new DH_ApplControl(*this);
}


// Redefines the preprocess function.
void 	DH_ApplControl::preprocess()
{
	addField ("VersionNumber", BlobField<uint16>(1));
	addField ("Command", 	   BlobField<int16>(1));	// ACCmd
	addField ("ScheduleTime",  BlobField<int32>(1));	// time_t
	addField ("WaitTime",  	   BlobField<int32>(1));	// time_t
	addField ("Result", 	   BlobField<uint16>(1));

	createDataBlock();

}


//# ---------- private ----------
// forbit assignment operator
DH_ApplControl& 	DH_ApplControl::operator=(const DH_ApplControl& that) { 
	// TODO implement
	return (*this);
}

// Implement the initialisation of the pointers
void	DH_ApplControl::fillDataPointers() {
	itsVersionNumber = getData<uint16>("VersionNumber");
	itsCommand 		 = getData<int16> ("Command");
	itsScheduleTime  = &static_cast<time_t>(*(getData<int32> ("ScheduleTime")));
	itsWaitTime		 = &static_cast<time_t>(*(getData<int32> ("WaitTime")));
	itsResult 		 = getData<uint16>("Result");

	*itsVersionNumber = 0x0100;		// TODO define a constant WriteVersion
}


} // namespace ACC
} // namespace LOFAR

