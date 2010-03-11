//#  PythonControl.h: Controller for the PythonControl
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

#ifndef PYTHONCONTROL_H
#define PYTHONCONTROL_H

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


class PythonControl : public GCFTask
{
public:
	explicit PythonControl(const string& cntlrName);
	~PythonControl();

	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult initial_state 		 (GCFEvent& event, GCFPortInterface& port);
	// Connect to Python mode
   	GCFEvent::TResult waitForConnection_state(GCFEvent& event, GCFPortInterface& port);
	// Normal control mode. 
	GCFEvent::TResult operational_state		 (GCFEvent& event, GCFPortInterface& port);
	// Finishing mode. 
	GCFEvent::TResult finishing_state		 (GCFEvent& event, GCFPortInterface& port);
	
	// Interrupthandler for switching to finisingstate when exiting the program
	static void signalHandler (int	signum);
	void	    finish();

private:
	// avoid defaultconstruction and copying
	PythonControl();
	PythonControl(const PythonControl&);
   	PythonControl& operator=(const PythonControl&);

	bool 	_startPython (const string&	cntlrName,
						  int			obsID,
						  const string&	pythonHost,
						  const string&	parentService);
	void	_databaseEventHandler(GCFEvent&				event);

	// ----- datamembers -----
   	RTDBPropertySet*           	itsPropertySet;
	bool					  	itsPropertySetInitialized;

	// pointer to parent control task
	ParentControl*			itsParentControl;
	GCFITCPort*				itsParentPort;

	GCFTimerPort*			itsTimerPort;

	GCFTCPPort*				itsListener;

	GCFTCPPort*				itsPythonPort;
	string					itsPythonName;

	CTState::CTstateNr		itsState;

	// ParameterSet variables
	string					itsTreePrefix;
};

  }  //CEPCU
}  //LOFAR
#endif
