//#  HardwareMonitor.h: Controller for the BeamServer
//#
//#  Copyright (C) 2006
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

#ifndef STATIONCU_HARDWARE_MONITOR_H
#define STATIONCU_HARDWARE_MONITOR_H

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

//# GCF Includes
#include <GCF/RTDB/RTDB_PropertySet.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_TimerPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Event.h>

// forward declaration

namespace LOFAR {
	namespace StationCU {

using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFTCPPort;
using	GCF::TM::GCFEvent;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTask;
using	GCF::RTDB::RTDBPropertySet;


class HardwareMonitor : public GCFTask
{
public:
	explicit HardwareMonitor(const string& cntlrName);
	~HardwareMonitor();

	// Interrupthandler for switching to finishingstate when exiting the program.
	static void sigintHandler (int signum);
	void finish();

private:
	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult initial_state   		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult connect2RSP   		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult askConfiguration 		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult createPropertySets	 (GCFEvent& e, GCFPortInterface& p);

   	GCFEvent::TResult askVersion    		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult askRSPinfo	  		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult askRCUinfo			 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult waitForNextCycle		 (GCFEvent& e, GCFPortInterface& p);

   	GCFEvent::TResult finishing_state  		 (GCFEvent& e, GCFPortInterface& p);

	// avoid defaultconstruction and copying
	HardwareMonitor();
	HardwareMonitor(const HardwareMonitor&);
   	HardwareMonitor& operator=(const HardwareMonitor&);

   	void _disconnectedHandler(GCFPortInterface& port);

	// Data members
	RTDBPropertySet*			itsOwnPropertySet;

	GCFTimerPort*				itsTimerPort;

	GCFTCPPort*					itsRSPDriver;

	uint32						itsPollInterval;

	uint32						itsNrRCUs;
	uint32						itsNrRSPboards;
	uint32						itsNrSubracks;
	uint32						itsNrCabinets;

	vector<RTDBPropertySet*>	itsCabinets;
	vector<RTDBPropertySet*>	itsSubracks;
	vector<RTDBPropertySet*>	itsRSPs;
	vector<RTDBPropertySet*>	itsRCUs;
};

  };//StationCU
};//LOFAR
#endif
