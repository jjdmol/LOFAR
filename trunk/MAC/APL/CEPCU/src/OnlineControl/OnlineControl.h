//#  OnlineControl.h: Controller for the OnlineControl
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

#ifndef ONLINECONTROL_H
#define ONLINECONTROL_H

//# Includes
#include <boost/shared_ptr.hpp>

//# GCF Includes
#include <GCF/PAL/GCF_MyPropertySet.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_ITCPort.h>
#include <GCF/TM/GCF_TimerPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Event.h>

//# local includes
#include <APL/APLCommon/PropertySetAnswerHandlerInterface.h>
#include <APL/APLCommon/PropertySetAnswer.h>
#include <APL/APLCommon/APLCommonExceptions.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/ParentControl.h>
#include <APL/APLCommon/CTState.h>

#include <CEPApplicationManager.h>

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_datetime.h>
#include <Common/LofarLogger.h>

//# ACC Includes
#include <APS/ParameterSet.h>

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


class OnlineControl : public GCFTask,
                      public APLCommon::PropertySetAnswerHandlerInterface,
                      public CEPApplicationManagerInterface
{
public:
	explicit OnlineControl(const string& cntlrName);
	~OnlineControl();

   	// PropertySetAnswerHandlerInterface method
   	void handlePropertySetAnswer(GCFEvent& answer);

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
    void    appBooted(uint16 result);
    void    appDefined(uint16 result);
    void    appInitialized(uint16 result);
    void    appRunDone(uint16 result);
    void    appPaused(uint16 result);
    void    appQuitDone(uint16 result);
    void    appSnapshotDone(uint16 result);
    void    appRecovered(uint16 result);
    void    appReinitialized(uint16 result);
    void    appReplaced(uint16 result);
    string  appSupplyInfo(const string& keyList);
    void    appSupplyInfoAnswer(const string& answer);
  
private:
	// avoid defaultconstruction and copying
	OnlineControl();
	OnlineControl(const OnlineControl&);
   	OnlineControl& operator=(const OnlineControl&);

	uint16_t doClaim(const string& cntlrName);
	uint16_t doPrepare(const string& cntlrName);
	void	 doRelease();
	void     finishController(uint16_t result);
   	void	 _connectedHandler(GCFPortInterface& port);
   	void	 _disconnectedHandler(GCFPortInterface& port);
	void	 setState(CTState::CTstateNr     newState);

   	typedef boost::shared_ptr<GCF::PAL::GCFMyPropertySet> GCFMyPropertySetPtr;

   	APLCommon::PropertySetAnswer  itsPropertySetAnswer;
   	GCFMyPropertySetPtr           itsPropertySet;
	bool						  itsPropertySetInitialized;

	// pointer to parent control task
	ParentControl*			itsParentControl;
	GCFITCPort*				itsParentPort;

	GCFTimerPort*			itsTimerPort;

    CEPApplicationManager   itsCepApplication;
    ACC::APS::ParameterSet  itsCepAppParams;
    ACC::APS::ParameterSet  itsResultParams;

	CTState::CTstateNr		itsState;

	// ParameterSet variables
	string					itsTreePrefix;
	uint32					itsInstanceNr;
	ptime					itsStartTime;
	ptime					itsStopTime;
	uint32					itsClaimPeriod;
	uint32					itsPreparePeriod;
};

  };//CEPCU
};//LOFAR
#endif
