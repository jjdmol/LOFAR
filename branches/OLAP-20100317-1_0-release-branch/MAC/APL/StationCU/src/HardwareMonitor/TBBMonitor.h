//#  TBBMonitor.h: Monitors the TBB hardware.
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
//#  $Id: TBBMonitor.h 10461 2007-08-23 22:44:03Z overeem $

#ifndef STATIONCU_TBB_MONITOR_H
#define STATIONCU_TBB_MONITOR_H

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_bitset.h>

//# GCF Includes
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/RTDB_PropertySet.h>
#include <APL/TBB_Protocol/TBB_Protocol.ph>

// forward declaration

namespace LOFAR {
	namespace StationCU {

using	MACIO::GCFEvent;
using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFTCPPort;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTask;
using	GCF::RTDB::RTDBPropertySet;


class TBBMonitor : public GCFTask
{
public:
	explicit TBBMonitor(const string& cntlrName);
	~TBBMonitor();

private:
	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult initial_state   		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult connect2TBB   		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult askConfiguration 		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult createPropertySets	 (GCFEvent& e, GCFPortInterface& p);

   	GCFEvent::TResult askVersion    		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult askSizeInfo	  		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult askFlashInfo	  		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult askTBBinfo	  		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult askRCUinfo			 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult askRCUSettings		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult waitForNextCycle		 (GCFEvent& e, GCFPortInterface& p);

   	GCFEvent::TResult finish_state  		 (GCFEvent& e, GCFPortInterface& p);

	// avoid defaultconstruction and copying
	TBBMonitor();
	TBBMonitor(const TBBMonitor&);
   	TBBMonitor& operator=(const TBBMonitor&);

   	void _disconnectedHandler(GCFPortInterface& port);
	string	TBBRCUstate(char	stateCode) ;

	// Data members
	RTDBPropertySet*			itsOwnPropertySet;

	GCFTimerPort*				itsTimerPort;

	GCFTCPPort*					itsTBBDriver;

	uint32						itsPollInterval;

	uint32						itsNrRCUs;
	uint32						itsNrTBboards;
	bitset<MAX_N_TBBOARDS>		itsBoardMask;

	vector<RTDBPropertySet*>	itsTBBs;
	vector<RTDBPropertySet*>	itsRCUs;
};

  };//StationCU
};//LOFAR
#endif
