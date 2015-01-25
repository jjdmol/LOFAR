//#  GPA_Controller.h: main class of the Property Agent
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

#ifndef GPA_CONTROLLER_H
#define GPA_CONTROLLER_H

#include <GPA_Defines.h>
#include <GPA_Converter.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/PAL/GCF_PVSSPort.h>

// This is the main class of the Property Agent. It uses a number of helper 
// classes to manage PML requests, registered scopes and use counts of created 
// properties. The assigned port provider supports the possibility to accept 
// more than one connect request from different clients (PML).

namespace LOFAR {
 namespace GCF {
  namespace PAL {

class GPAPropSetSession;

class GPAController : public TM::GCFTask
{
public:
	GPAController();
	virtual ~GPAController();
  
private: 
	// state methods
	TM::GCFEvent::TResult startup_state(TM::GCFEvent& e, TM::GCFPortInterface& p);
	TM::GCFEvent::TResult waiting_state(TM::GCFEvent& e, TM::GCFPortInterface& p);

	// helper methods
    friend class GPAPropSetSession;

    void acceptConnectRequest(TM::GCFPortInterface& p);
    void mayDeleted			 (GPAPropSetSession& 	session);
    void closingPortFinished (GPAPropSetSession&	pss, TM::GCFPortInterface& p);
    void propSetIdle		 (GPAPropSetSession&	idlePropSet);    
    void emptyGarbage		 ();
    GPAPropSetSession*	findPropSet	(const string& scope) const;
    
	// data members
    typedef map<string /*scope*/, GPAPropSetSession*> TPropSetSessions;
    TPropSetSessions         	_propSetSessions;			// the actual PSses
    list<GPAPropSetSession*> 	_propSetSessionsGarbage;	// to be deleted
    
    typedef map<TM::GCFPortInterface* /*pPortToDelete*/, list<GPAPropSetSession*> /*pendingPSs*/> TClosedPorts;
    // pendingPSs = propsets, which are busy with the close port procedure
    
    list<TM::GCFPortInterface*> _pmlPorts;					// conn. with clients
    TClosedPorts      			_pmlPortGarbage;			// ports to delete
    
	TM::GCFTCPPort    			_pmlPortProvider;			// TCP listener
    PAL::GCFPVSSPort  			_distPmlPortProvider;		// PVSS DP 'listener'
    
	// admin. data members
    unsigned long     			_garbageTimerId;
    unsigned long     			_queueTimerId;
    GPAConverter      			_converter;   
};

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

#endif
