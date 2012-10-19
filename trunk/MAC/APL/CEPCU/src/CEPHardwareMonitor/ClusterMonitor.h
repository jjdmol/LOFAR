//#  ClusterMonitor.h: Monitors the Cluster hardware.
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
//#  $Id: ClusterMonitor.h 10461 2007-08-23 22:44:03Z overeem $

#ifndef CEPCU_CLUSTER_MONITOR_H
#define CEPCU_CLUSTER_MONITOR_H

//# Common Includes
#include <blitz/array.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/LofarConstants.h>

//# GCF Includes
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/RTDB_PropertySet.h>
#include <GCF/RTDB/DPservice.h>

// forward declaration

namespace LOFAR {
	namespace CEPCU {

using	MACIO::GCFEvent;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFTCPPort;
using	GCF::TM::GCFTask;
using	GCF::RTDB::RTDBPropertySet;
using	GCF::RTDB::DPservice;


class ClusterMonitor : public GCFTask
{
public:
	explicit ClusterMonitor(const string& cntlrName);
	~ClusterMonitor();

private:
	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult initial_state    (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult getClusterState (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult waitForNextCycle (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult finish_state	   (GCFEvent& e, GCFPortInterface& p);

	void _analyzeClusterResult(vector<char>	result, size_t	length);
	string _clusterNodeName(int nodeNr);

	// avoid defaultconstruction and copying
	ClusterMonitor();
	ClusterMonitor(const ClusterMonitor&);
   	ClusterMonitor& operator=(const ClusterMonitor&);

	// Data members
	RTDBPropertySet*			itsOwnPropertySet;

	GCFTimerPort*				itsTimerPort;
	DPservice*					itsDPservice;

	string						itsClusterNameMask;
	string						itsClusterNetwork;
	int							itsFirstClusterNode;
	int							itsLastClusterNode;

	uint32						itsPollInterval;
	vector<int>					itsLastState;
};

  };//CEPCU
};//LOFAR
#endif
