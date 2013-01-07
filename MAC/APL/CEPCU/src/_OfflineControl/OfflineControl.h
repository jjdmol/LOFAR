//#  OfflineControl.h: Controller for the OfflineControl
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

#ifndef OFFLINECONTROL_H
#define OFFLINECONTROL_H

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_datetime.h>
#include <Common/LofarLogger.h>

//# ACC Includes
#include <Common/ParameterSet.h>

//# GCF Includes
#include <GCF/RTDB/RTDBPropertySet.h>
#include <GCF/TM/GCF_Control.h>

//# local includes
#include <APL/APLCommon/APLCommonExceptions.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/ParentControl.h>
#include <APL/APLCommon/CTState.h>

#include <CEPApplicationManager.h>

// forward declaration

namespace LOFAR {
	namespace CEPCU {

using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFITCPort;
using	GCF::TM::GCFPort;
using	GCF::TM::GCFEvent;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTask;
using	APLCommon::ParentControl;


class OfflineControl : public GCFTask,
                      public CEPApplicationManagerInterface
{
public:
	explicit OfflineControl(const string& cntlrName);
	~OfflineControl();

	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult initial_state (GCFEvent& e, 
									 GCFPortInterface& p);
	
	// Normal control mode. 
   	GCFEvent::TResult active_state  (GCFEvent& e, 
									 GCFPortInterface& p);

	// Finishing mode. 
	GCFEvent::TResult finished_state(GCFEvent& event, 
									 GCFPortInterface& port);

protected: // implemenation of abstract CEPApplicationManagerInterface methods
    void    appBooted(const string& procName, uint16 result);
    void    appDefined(const string& procName, uint16 result);
    void    appInitialized(const string& procName, uint16 result);
    void    appRunDone(const string& procName, uint16 result);
    void    appPaused(const string& procName, uint16 result);
    void    appQuitDone(const string& procName, uint16 result);
    void    appSnapshotDone(const string& procName, uint16 result);
    void    appRecovered(const string& procName, uint16 result);
    void    appReinitialized(const string& procName, uint16 result);
    void    appReplaced(const string& procName, uint16 result);
    string  appSupplyInfo(const string& procName, const string& keyList);
    void    appSupplyInfoAnswer(const string& procName, const string& answer);
  
private:
	// avoid defaultconstruction and copying
	OfflineControl();
	OfflineControl(const OfflineControl&);
   	OfflineControl& operator=(const OfflineControl&);

	uint16_t doClaim(const string& cntlrName);
	uint16_t doPrepare(const string& cntlrName);
	void prepareProcess(const string& cntlrName, const string& procName, const time_t startTime);
	void	 doRelease();
	void     finishController(uint16_t result);
   	void	 _connectedHandler(GCFPortInterface& port);
   	void	 _disconnectedHandler(GCFPortInterface& port);
	void	 setState(CTState::CTstateNr     newState);

   	RTDBPropertySet*         	itsPropertySet;
	bool					  	itsPropertySetInitialized;

	// pointer to parent control task
	ParentControl*			itsParentControl;
	GCFITCPort*				itsParentPort;

	GCFTimerPort*			itsTimerPort;

    map<string, CEPApplicationManager*>	itsCepApplications;
    map<string, ParameterSet>	itsCepAppParams;
    ParameterSet				itsResultParams;
	map<string, vector<string> >		itsProcessDependencies;
	map<string, time_t>					itsCepAppStartTimes;

	CTState::CTstateNr		itsState;

	// ParameterSet variables
	string					itsTreePrefix;
	uint32					itsInstanceNr;
	ptime                   itsStartTime;
	ptime					itsStopTime;
	uint32					itsClaimPeriod;
	uint32					itsPreparePeriod;
	string                  itsCntlrName;
};

  };//CEPCU
};//LOFAR
#endif
