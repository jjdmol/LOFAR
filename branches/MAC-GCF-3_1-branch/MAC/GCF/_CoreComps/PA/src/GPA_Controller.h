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
#include <GPA_UsecountManager.h>
#include <GPA_RequestManager.h>
#include <GPA_ScopeManager.h>
#include <GPA_APCFileReader.h>
#include <GCF/GCF_Task.h>
#include <GCF/GCF_TCPPort.h>

/**
   This is the main class of the Property Agent. It uses a number of helper 
   classes to manage PML requests, registered scopes and use counts of created 
   properties. The assigned port provider supports the possibility to accept 
   more than one connect request from different clients (PML).
*/

class GCFEvent;
class GCFPortInterface; 

class GPAController : public GCFTask
{
	public:
		GPAController();
		virtual ~GPAController();
  
	private: // GPAUsecountManager call back methods
    friend class GPAUsecountManager;
		void propertiesCreated(list<string>& propList);
    void propertiesDeleted(list<string>& propList);
    void allPropertiesDeletedByScope();
    void allPropertiesDeleted();
	
	private: // helper methods
    friend class GPAScopeManager;
    void doNextRequest();
    bool mayContinue(GCFEvent& e, GCFPortInterface& p);
    void loadAPC(GCFEvent& e);
    void apcLoaded(TPAResult result);
    void unloadAPC(GCFEvent& e);
    void apcUnloaded(TPAResult result);
    void reloadAPC(GCFEvent& e);
    void unregisterScope(GCFEvent& e);
    void propertiesLinked(GCFEvent& e);
    void propertiesUnlinked(GCFEvent& e);
    void sendAPCActionResponse(GCFEvent& e);
    
	private: // state methods
		GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface& p);
		GCFEvent::TResult connected(GCFEvent& e, GCFPortInterface& p);

	private: // data members
		GPAUsecountManager 	_usecountManager;
		GPARequestManager 	_requestManager;
		GPAScopeManager 		_scopeManager;

    list<GCFPortInterface*> _pmlPorts;		
		GCFTCPPort					_pmlPortProvider;
    GPAAPCFileReader    _apcFileReader;   
    
  private: // admin. data members
    string              _curApcName;
    string              _curScope;
    GCFPortInterface*   _curRequestPort;
    TPAResult           _curResult;
    bool                _isBusy;
    bool                _isRegistered;
    unsigned int        _counter;  
};

#endif
