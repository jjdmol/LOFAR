//# DH_ApplControl.cc: Implements the Application Controller command protocol.
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

//# Includes
#include <ALC/DH_ApplControl.h>


namespace LOFAR {
  namespace ACC {
    namespace ALC {

// Constructor
DH_ApplControl::DH_ApplControl() :
	DataHolder ("", "DH_ApplControl"),
	itsVersionNumber(0),
	itsCommand(0),
	itsScheduleTime(0),
	itsResult(0)
{
	LOG_TRACE_OBJ("DH_applControl()");
	setExtraBlob ("Extra", 1);
}


// Destructor
DH_ApplControl::~DH_ApplControl() 
{
	LOG_TRACE_OBJ("~DH_applControl()");
}

// Copying is allowed.
DH_ApplControl::DH_ApplControl(const DH_ApplControl& that) :
	DataHolder(that),
	itsVersionNumber(that.itsVersionNumber),
	itsCommand(that.itsCommand),
	itsScheduleTime(that.itsScheduleTime),
	itsResult(that.itsResult)
{
	setExtraBlob ("Extra", 1);
}

DH_ApplControl*		DH_ApplControl::clone() const
{
	return new DH_ApplControl(*this);
}

DH_ApplControl*		DH_ApplControl::makeDataCopy() const
{
	DH_ApplControl*		newDHAC = new DH_ApplControl;
	newDHAC->init();
	newDHAC->setCommand     (getCommand());
	newDHAC->setScheduleTime(getScheduleTime());
	newDHAC->setWaitTime    (getWaitTime());
	newDHAC->setOptions     (getOptions());
	newDHAC->setProcList    (getProcList());
	newDHAC->setNodeList    (getNodeList());
	newDHAC->setResult      (getResult());
	newDHAC->pack();

	return (newDHAC);
}	

// Redefines the init function.
void 	DH_ApplControl::init()
{
	LOG_TRACE_FLOW("DH_ApplControl:init()");

	initDataFields();

	addField ("VersionNumber", BlobField<uint16>(1));
	addField ("Command", 	   BlobField<int16>(1));	// ACCmd
	addField ("ScheduleTime",  BlobField<int32>(1));	// time_t
	addField ("WaitTime",  	   BlobField<int32>(1));	// time_t
	addField ("Result", 	   BlobField<uint16>(1));

	createDataBlock();

}

// Redefine the pack function.
void	DH_ApplControl::pack()
{
	LOG_TRACE_RTTI("DH_ApplControl:pack()");

	BlobOStream&	bos = createExtraBlob();		// attached to dataholder

	bos <<	itsOptions;
	bos <<	itsProcList;
	bos <<	itsNodeList;
	
	DataHolder::pack();
}

// Redefine the unpack function.
void	DH_ApplControl::unpack()
{
	LOG_TRACE_RTTI("DH_ApplControl:unpack()");

	DataHolder::unpack();

	int32			version;
	bool			found;
	BlobIStream&	bis = getExtraBlob(found, version);
	ASSERTSTR (found, "DH_ApplControl::read has no extra blob");

	bis >>	itsOptions;
	bis >>	itsProcList;
	bis >>	itsNodeList;
	bis.getEnd();
}


//# ---------- private ----------

// Implement the initialisation of the pointers
void	DH_ApplControl::fillDataPointers() {
	LOG_TRACE_FLOW("DH_ApplControl:fillDataPointers()");

	itsVersionNumber = getData<uint16>("VersionNumber");
	itsCommand 		 = getData<int16> ("Command");
	itsScheduleTime  = getData<int32> ("ScheduleTime");
	itsWaitTime		 = getData<int32> ("WaitTime");
	itsResult 		 = getData<uint16>("Result");

	*itsVersionNumber = 0x0100;		// TODO define a constant WriteVersion
}


    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

