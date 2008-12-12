//#  VHECRTask.h: Controller for the TBBDriver
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

#ifndef STATIONCU_TBBCONTROL_VHECRTASK_H
#define STATIONCU_TBBCONTROL_VHECRTASK_H

//# Common Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>
#include <Common/lofar_datetime.h>

//# local includes
#include "TBBTrigger.h"
#include "TBBReadCmd.h"

// forward declaration


namespace LOFAR {
	namespace StationCU {

class VHECRTask
{
public:
	VHECRTask();
	~VHECRTask();

	// define responsefunctionType
	void VHECRTask::readTBBdata(vector<TBBReadCmd>	cmdVector);
	void addTrigger(const TBBTrigger&	trigger);

private:
	// avoid defaultconstruction and copying
	VHECRTask(const VHECRTask&);
   	VHECRTask& operator=(const VHECRTask&);

	// remote function to call for saving triggers
	uint32					itsNrTriggers;			// just for statistics
	bool					itsInitialized;
	vector<TBBReadCmd>		itsCommandVector;		// used as temporarely buffer

};

  };//StationCU
};//LOFAR
#endif
