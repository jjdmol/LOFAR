//#  ECMonitor.h: Monitors the TBB hardware.
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

#ifndef STATIONCU_EC_MONITOR_H
#define STATIONCU_EC_MONITOR_H

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_bitset.h>

//# GCF Includes
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/RTDB_PropertySet.h>
#include <EC_Protocol.ph>

// forward declaration

namespace LOFAR {
	namespace StationCU {

using	MACIO::GCFEvent;
using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFTCPPort;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTask;
using	GCF::RTDB::RTDBPropertySet;


class ECMonitor : public GCFTask
{
public:
	explicit ECMonitor(const string& cntlrName);
	~ECMonitor();

private:
	// During the initial state all connections with the other programs are made.
	GCFEvent::TResult initial_state   		 (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult connect2EC     		 (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult createPropertySets	 (GCFEvent& e, GCFPortInterface& p);
	
	GCFEvent::TResult askSettings           (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult askStatus       		 (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult waitForNextCycle		 (GCFEvent& e, GCFPortInterface& p);

	GCFEvent::TResult finish_state  		 (GCFEvent& e, GCFPortInterface& p);
	
	

	// avoid defaultconstruction and copying
	ECMonitor();
	ECMonitor(const ECMonitor&);
	ECMonitor& operator=(const ECMonitor&);

	void _disconnectedHandler(GCFPortInterface& port);
	
	string ctrlMode(int16 mode);
	
	// Data members
	RTDBPropertySet*			itsOwnPropertySet;

	GCFTimerPort*				itsTimerPort;

	GCFTCPPort*					itsECPort;

	uint32						itsPollInterval;
	
	int							itsNrCabs;
	
	int							itsNrSystemCabs;
	
	RTDBPropertySet*			itsStation;
	
	vector<RTDBPropertySet*>	itsCabs;
};


class RawEvent
{
public:
	static GCFEvent::TResult dispatch(GCFTask& task, GCFPortInterface& port);
};

	};//StationCU
};//LOFAR
#endif
