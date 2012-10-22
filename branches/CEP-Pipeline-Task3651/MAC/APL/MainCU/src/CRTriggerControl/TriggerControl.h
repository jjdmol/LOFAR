//#  TriggerControl.h: Distribution of (external) triggerrequest to the stations
//#
//#  Copyright (C) 2011
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
//#  $Id: TriggerControl.h 17961 2011-05-04 15:02:32Z overeem $

#ifndef TriggerControl_H
#define TriggerControl_H

//# Common Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>
#include <Common/lofar_datetime.h>

//# GCF Includes
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/RTDB_PropertySet.h>
#include <GCF/RTDB/GCF_RTDBPort.h>
#include <GCF/RTDB/DPservice.h>

// forward declaration

namespace LOFAR {
	using	MACIO::GCFEvent;
	using	GCF::TM::GCFTimerPort;
	using	GCF::TM::GCFPortInterface;
	using	GCF::TM::GCFTCPPort;
	using	GCF::TM::GCFTask;
	using	GCF::RTDB::RTDBPropertySet;
	using	GCF::RTDB::DPservice;
	using	GCF::RTDB::GCFRTDBPort;
	namespace MainCU {

class TriggerControl : public GCFTask
{
public:
	explicit TriggerControl(const string& cntlrName);
	~TriggerControl();

	// During this state the top DP LOFAR_ObsSW_<observation> is created
   	GCFEvent::TResult initial_state (GCFEvent& e, GCFPortInterface& p);
	
	// During this state all connections with the other programs are made.
   	GCFEvent::TResult attach2MACScheduler (GCFEvent& e, GCFPortInterface& p);
	
	// During this state all connections with the other programs are made.
   	GCFEvent::TResult openPublisher (GCFEvent& e, GCFPortInterface& p);
	
	// Normal control mode. 
   	GCFEvent::TResult operational_state (GCFEvent& e, GCFPortInterface& p);

	// Terminating mode. 
   	GCFEvent::TResult finishing_state(GCFEvent& e, GCFPortInterface& p);

	// Interrupt handler for switching to finishing_state when exiting program.
	static void sigintHandler (int signum);
	void finish ();
	void abortTrigger ();

private:
	// avoid defaultconstruction and copying
	TriggerControl();
	TriggerControl(const TriggerControl&);
   	TriggerControl& operator=(const TriggerControl&);


	void _presetAdministration();
	void _cleanupAdministration();
	bool _addObservation(int obsID);
    
	void _handleQueryEvent(GCFEvent& event);
   	void _connectedHandler(GCFPortInterface& port);
   	void _disconnectedHandler(GCFPortInterface& port);
   	void _databaseEventHandler(GCFEvent& answer);
    
    void _CRstopHandler(GCFEvent& event, GCFPortInterface& port);
    void _CRreadHandler(GCFEvent& event, GCFPortInterface& port);
    void _CRrecordHandler(GCFEvent& event, GCFPortInterface& port);
    void _CRstopDumpsHandler(GCFEvent& event, GCFPortInterface& port);
    void _CRcepSpeedHandler(GCFEvent& event, GCFPortInterface& port);

	// ----- data members -----
   	RTDBPropertySet*		itsPropertySet;			// my own propset.
	DPservice*				itsDPservice;
	int						itsSubscriptionID;
	GCFRTDBPort*			itsPublisher;
	GCFTCPPort*				itsListener;
	GCFTCPPort*             itsClient;
	GCFTimerPort*			itsTimerPort;

	typedef struct ObsInfo {
		vector<string>	stationList;
		int				obsID;
		bool			updated;
		ptime			startTime;
		ptime			stopTime;
	} ObsInfo;
	map <int /*obsID*/, ObsInfo>		itsObservations;
};

  };//MainCU
};//LOFAR
#endif
