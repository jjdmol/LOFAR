//#  RSPMonitor.h: Monitors the RSP hardware.
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
//#  $Id: RSPMonitor.h 10461 2007-08-23 22:44:03Z overeem $

#ifndef STATIONCU_RSP_MONITOR_H
#define STATIONCU_RSP_MONITOR_H

//# Common Includes
#include <blitz/array.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

//# GCF Includes
#include <APL/APLCommon/AntennaMapper.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/RTDB_PropertySet.h>
#include <GCF/RTDB/DPservice.h>

// forward declaration

namespace LOFAR {
	namespace StationCU {

using	MACIO::GCFEvent;
using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFTCPPort;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTask;
using	GCF::RTDB::RTDBPropertySet;
using	GCF::RTDB::DPservice;
using	APLCommon::AntennaMapper;


class RSPMonitor : public GCFTask
{
public:
	explicit RSPMonitor(const string& cntlrName);
	~RSPMonitor();

private:
	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult initial_state   		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult connect2RSP   		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult askConfiguration 		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult createPropertySets	 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult subscribeToRCUs		 (GCFEvent& e, GCFPortInterface& p);

   	GCFEvent::TResult askVersion    		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult askRSPinfo	  		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult askTDstatus	  		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult askSPUstatus	  		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult askRCUinfo			 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult waitForNextCycle		 (GCFEvent& e, GCFPortInterface& p);

   	GCFEvent::TResult finish_state  		 (GCFEvent& e, GCFPortInterface& p);

	// avoid defaultconstruction and copying
	RSPMonitor();
	RSPMonitor(const RSPMonitor&);
   	RSPMonitor& operator=(const RSPMonitor&);

	// helper functions
   	void _disconnectedHandler(GCFPortInterface& port);
	void _doQueryChanged	 (GCFEvent&			event);

	// Data members
	RTDBPropertySet*			itsOwnPropertySet;

	GCFTimerPort*				itsTimerPort;
	GCFTCPPort*					itsRSPDriver;
	DPservice*					itsDPservice;

	uint32						itsPollInterval;

	uint32						itsNrRCUs;
	uint32						itsNrRSPboards;
	uint32						itsNrSubracks;
	uint32						itsNrCabinets;
	uint32						itsNrLBAs;
	uint32						itsNrHBAs;

	vector<RTDBPropertySet*>	itsCabinets;
	vector<RTDBPropertySet*>	itsSubracks;
	vector<RTDBPropertySet*>	itsRSPs;
	vector<RTDBPropertySet*>	itsRCUs;

	blitz::Array<uint,1>		itsRCUstates;		// actual status of the RCUs
	blitz::Array<bool,2>		itsRCUInputStates;	// enable state of the three RCU inputs
	int							itsRCUquery;		// ID of the PVSS query
	AntennaMapper*				itsAntMapper;

};

  };//StationCU
};//LOFAR
#endif
