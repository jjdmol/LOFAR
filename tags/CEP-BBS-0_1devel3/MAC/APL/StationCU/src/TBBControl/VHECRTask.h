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

//# GCF Includes
#include <GCF/TM/GCF_Control.h>

//# local includes
#include "TBBTrigger.h"
#include "TBBReadCmd.h"

// forward declaration

namespace LOFAR {
	namespace StationCU {

using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFEvent;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTask;


class VHECRTask : public GCFTask
{
public:
	explicit VHECRTask(const string& cntlrName);
	~VHECRTask();

	// define responsefunctionType
	typedef	void (*saveFunctionType) (vector<TBBReadCmd>);

	void	addTrigger(const TBBTrigger&	trigger);
	void	setSaveFunction(saveFunctionType	saveFunction);

private:
	// Wait till maintask has installed my responsefunction
   	GCFEvent::TResult initial_state (GCFEvent& e, GCFPortInterface& p);

	// Normal operationmode
   	GCFEvent::TResult operational  (GCFEvent& e, GCFPortInterface& p);

	// Quiting, shutdown connections, send FINISH and quit
   	GCFEvent::TResult quiting_state (GCFEvent& e, GCFPortInterface& p);

	// avoid defaultconstruction and copying
	VHECRTask();
	VHECRTask(const VHECRTask&);
   	VHECRTask& operator=(const VHECRTask&);

	// pointer to timers
	GCFTimerPort*			itsTimerPort;

	// remote function to call for saving triggers
	saveFunctionType		itsSaveFunction;

	// [TEST] datamembers
	uint32					itsMinResponseTime;		// for simulating varying behaviour
	uint32					itsMaxResponseTime;
	uint32					itsNrTriggers;			// just for statistics
	bool					itsInitialized;
	vector<TBBReadCmd>		itsCommandVector;		// used as temporarely buffer

};

  };//StationCU
};//LOFAR
#endif
