//  SoftwareMonitor.h: Monitors the Software hardware.
//
//  Copyright (C) 2008
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#ifndef RTDBDAEMONS_SOFTWARE_MONITOR_H
#define RTDBDAEMONS_SOFTWARE_MONITOR_H

// Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

// GCF Includes
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/RTDB_PropertySet.h>
#include <GCF/RTDB/DPservice.h>

// forward declaration

namespace LOFAR {
	using	MACIO::GCFEvent;
	using	GCF::TM::GCFTimerPort;
	using	GCF::TM::GCFTCPPort;
	using	GCF::TM::GCFPortInterface;
	using	GCF::TM::GCFTask;
	using	GCF::RTDB::RTDBPropertySet;
	using	GCF::RTDB::DPservice;
	namespace RTDBDaemons {

class SoftwareMonitor : public GCFTask
{
public:
	explicit SoftwareMonitor(const string& cntlrName);
	~SoftwareMonitor();

private:
	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult initial_state		(GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult readSWlevels		(GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult checkPrograms		(GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult waitForNextCycle	(GCFEvent& e, GCFPortInterface& p);

   	GCFEvent::TResult finish_state		(GCFEvent& e, GCFPortInterface& p);

	// avoid defaultconstruction and copying
	SoftwareMonitor();
	SoftwareMonitor(const SoftwareMonitor&);
   	SoftwareMonitor& operator=(const SoftwareMonitor&);

	// helper functions
	void _buildProcessMap();

	// Data members
	RTDBPropertySet*		itsOwnPropertySet;
	GCFTimerPort*			itsTimerPort;
	DPservice*				itsDPservice;

	struct Process {
		// static information
		string		name;
		int			level;
		bool		mustBroot;
		bool		runsUnderMPI;
		bool		permSW;
		// actual info
		int			pid;			// TODO: make is suitable for multiple instances
		int			errorCnt;
	};
	vector<Process>			itsLevelList;		// list that represents swlevel.conf

	typedef multimap<string, int>	processMap_t;
	multimap<string, int>	itsProcessMap;		// list that represents 'ps -ef'

	// read from configfile
	int						itsPollInterval;
	int						itsSuspThreshold;
	int						itsBrokenThreshold;
	int						itsMaxRestartRetries;

};

 }; // RTDBDaemons
}; // LOFAR
#endif
