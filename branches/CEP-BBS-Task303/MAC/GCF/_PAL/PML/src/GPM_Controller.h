//#  GPM_Controller.h: singleton class; bridge between controller application 
//#                    and Property Agent
//#
//#  Copyright (C) 2002-2003
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

#ifndef GPM_CONTROLLER_H
#define GPM_CONTROLLER_H

#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_Handler.h>
#include <GCF/PAL/GCF_PVSSPort.h>
#include "GPM_Defines.h"
#include <GCF/Protocols/PA_Protocol.ph>

/**
   This singleton class forms the bridge between the PML API classes and the PA. 
   It is a hidden task with its own state machine in each Application, which 
   wants to be part of the MAC subsystem of LOFAR. It will be created at the 
   moment a service of PML will be requested by the Application (like load 
   property set or load APC).
*/

namespace LOFAR {
 namespace GCF {
  namespace Common {
	class GCFPValue;
  }
  namespace TM {
	class TM::GCFEvent;
	class TM::GCFPortInterface;
  }
  namespace PAL {
	class GCFPropertySet;
	class GCFMyPropertySet;
	class GCFExtPropertySet;
	class GPMHandler;
	class GCFSysConnGuard;

class GPMController : public TM::GCFTask
{
public:
	~GPMController ();
	static GPMController* instance(bool temporary = false);
	static void release();

	// member functions
	TPMResult loadPropSet 	   (GCFExtPropertySet& propSet);
	TPMResult unloadPropSet    (GCFExtPropertySet& propSet);
	TPMResult configurePropSet (GCFPropertySet&    propSet, const string& apcName);
	void 	  deletePropSet    (const GCFPropertySet& propSet);

	TPMResult registerScope   (GCFMyPropertySet& propSet);
	TPMResult unregisterScope (GCFMyPropertySet& propSet);

	void propertiesLinked   (const string& scope, TPAResult result);
	void propertiesUnlinked (const string& scope, TPAResult result);

private:
	friend class GPMHandler;
	GPMController ();

	// state methods
	TM::GCFEvent::TResult initial   (TM::GCFEvent& e, TM::GCFPortInterface& p);
	TM::GCFEvent::TResult connected (TM::GCFEvent& e, TM::GCFPortInterface& p);

	// helper methods for administration of (future) actions.
	// Define a struct the can contain the action that must be executed.
	typedef struct Action {
		GCFPropertySet* pPropSet;
		string 			apcName;
		unsigned short	signal;
		Action& operator= (const Action& other) {
			if (this != &other) {
				pPropSet = other.pPropSet;
				signal   = other.signal;
				apcName.replace(0, string::npos, other.apcName);
			}
			return *this;
		}      
	} TAction;
	// add the given action to the action queue
	uint16	registerAction  (TAction& action);
	// Constructs the full DPname of the PA that will handle the administration
	// of the given datapoint.
	string	getPAcommunicationDP(const string& fullDPname) const;
	bool 	checkDestination    (const string& PAcommDP) const;
	GCFPropertySet*   findPropSetInActionList(uint16 seqnr) const;
	GCFMyPropertySet* findMyPropSet			 (string& scope) const;

	// data members        
	typedef map<string /* scope */, GCFMyPropertySet*>  TMyPropertySets;
	typedef list<GCFExtPropertySet*>  TExtPropertySets;
	typedef map<unsigned short /*seqnr*/, TAction>  TActionSeqList;
	typedef struct {
		unsigned long linkedTimeOutId;
		unsigned long lastRetryTimerId;
	} TLinkTimers;  
	typedef map<GCFMyPropertySet*, TLinkTimers>  TLinkTimerList;

	TMyPropertySets 	_myPropertySets;
	TExtPropertySets 	_extPropertySets;
	TActionSeqList 		_actionSeqList;    
	TLinkTimerList 		_linkTimerList;    

	GCFTCPPort* 		itsPAport;
	GCFPVSSPort*		itsPADBport;
	GCFSysConnGuard*	_pSysConnGuard;

};

class GPMHandler : public TM::GCFHandler
{
public:
	~GPMHandler() { _pInstance = 0; }
	void workProc() {};
	void stop () {};

private:
	friend class GPMController;
	GPMHandler() {};

	static GPMHandler*	_pInstance;
	GPMController 		_controller;
};

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
