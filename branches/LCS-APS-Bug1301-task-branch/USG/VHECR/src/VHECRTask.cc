//#  VHECRTask.cc: Implementation of the VHECR algoritms
//#
//#  Copyright (C) 2007
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
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>

#include "VHECRTask.h"

namespace LOFAR {
	namespace StationCU {
	
//
// VHECRTask()
//
VHECRTask::VHECRTask() :
	itsNrTriggers		(0),
	itsInitialized		(false)
{
	LOG_DEBUG ("VHECR construction");
}


//
// ~VHECRTask()
//
VHECRTask::~VHECRTask()
{
	LOG_DEBUG ("VHECR destruction");
}

//
// readTBBdata(vector<TBBReadCmd>	cmdVector)
//
void VHECRTask::readTBBdata(vector<TBBReadCmd>	cmdVector)
{
	// for now we only print the info that comes in.
	vector<TBBReadCmd>::iterator	iter = cmdVector.begin();
	vector<TBBReadCmd>::iterator	end  = cmdVector.end();
	while (iter != end) {
		LOG_INFO_STR(*iter);
		iter++;
	}
}

//
// addTrigger(trigger)
//
// THIS IS WERE THE DEVELOPMENT SHOULD TAKE PLACE.
//
void VHECRTask::addTrigger(const TBBTrigger&	trigger)
{
	// [TEST] The contents of this function is pure test code, it adds the trigger to the command queue.
	uint32		sampleTime = 1000;
	uint32		prePages   = 1;
	uint32		postPages  = 2;
	itsCommandVector.push_back(TBBReadCmd(trigger.itsRcuNr, trigger.itsTime, sampleTime, prePages, postPages));	
	itsNrTriggers++;


	// All code for this event is [TEST] code
	if (!itsCommandVector.empty()) {
		readTBBdata(itsCommandVector);			// report that we want everything
		itsCommandVector.clear();					// clear buffer
	}
}


}; // StationCU
}; // LOFAR
