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

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_datetime.h>
#include <Common/LofarLogger.h>

//# ACC Includes
#include <Common/ParameterSet.h>

//# GCF Includes
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/RTDB_PropertySet.h>

//# local includes
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/ParentControl.h>
#include <APL/APLCommon/CTState.h>

#include <CEPApplMgr.h>

// forward declaration

namespace LOFAR {
	namespace CEPCU {

using	MACIO::GCFEvent;
using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFITCPort;
using	GCF::TM::GCFPort;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTask;
using	GCF::RTDB::RTDBPropertySet;
using	APLCommon::ParentControl;


class OnlineControl : public GCFTask,
                      public CEPApplMgrInterface
{
public:
	explicit OnlineControl(const string& cntlrName);
	~OnlineControl();

	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult initial_state (GCFEvent& e, 
									 GCFPortInterface& p);
	// Normal control mode. 
   	GCFEvent::TResult active_state  (GCFEvent& e, 
									 GCFPortInterface& p);
	// Finishing mode. 
	GCFEvent::TResult finishing_state(GCFEvent& event, 
									  GCFPortInterface& port);
	
	// Interrupthandler for switching to finisingstate when exiting the program
	static void signalHandler (int	signum);
	void	    finish();

protected: // implemenation of abstract CEPApplMgrInterface methods
    string  appSupplyInfo		(const string& procName, const string& keyList);
    void    appSupplyInfoAnswer (const string& procName, const string& answer);
	// A result of one of the applications was received, update the administration
	// off the controller and send the result to the parentcontroller if appropriate.
	void	appSetStateResult	 (const string&			procName, 
								  CTState::CTstateNr   	newState, 
								  uint16				result);

  
private:
	// avoid defaultconstruction and copying
	OnlineControl();
	OnlineControl(const OnlineControl&);
   	OnlineControl& operator=(const OnlineControl&);

	void	_doBoot();
	void	_doQuit();
	void   	_finishController	 (uint16_t 				result);
   	void	_connectedHandler	 (GCFPortInterface& 	port);
   	void	_disconnectedHandler (GCFPortInterface& 	port);
	void	_setState	  		 (CTState::CTstateNr	newState);
	void	_databaseEventHandler(GCFEvent&				event);

	// Send a command to all (or the first) applications.
	void	startNewState (CTState::CTstateNr		newState,
						   const string&			options);

	// typedefs for the internal adminsitration of all the Applications we control
	typedef boost::shared_ptr<CEPApplMgr> CEPApplMgrPtr;
    typedef	map<string, CEPApplMgrPtr>  			CAMmap;
	typedef map<string, CEPApplMgrPtr>::iterator	CAMiter;

	// Internal bookkeeping-finctions for the dependancy-order of the applications. 
	void	setApplOrder	(vector<string>&	anApplOrder);
	CAMiter	firstApplication(CTState::CTstateNr		aState);
	CAMiter	nextApplication();
	bool	hasNextApplication();
	void	noApplication();

	// ----- datamembers -----
   	RTDBPropertySet*           	itsPropertySet;
	bool					  	itsPropertySetInitialized;

	// pointer to parent control task
	ParentControl*			itsParentControl;
	GCFITCPort*				itsParentPort;

	GCFTimerPort*			itsTimerPort;

	CAMmap					itsCEPapplications;
    ParameterSet  itsResultParams;

	CTState::CTstateNr		itsState;

	bool					itsUseApplOrder;	// Applications depend?
	vector<string>			itsApplOrder;		// startOrder of the applications.
	string					itsCurrentAppl;		// current application we are handling.
	CTState::CTstateNr		itsApplState;		// state currently handled by apps.
	string					itsOptions;			// Current active option

	uint16					itsOverallResult;
	int16					itsNrOfAcks2Recv;

	// ParameterSet variables
	string					itsTreePrefix;
	uint32					itsInstanceNr;
	ptime					itsStartTime;
	ptime					itsStopTime;
	uint16					itsStopTimerID;
	uint16					itsFinishTimerID;
};

  };//CEPCU
};//LOFAR
#endif
