//#  GPA_Controller.h: 
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
#include <GPA_APC.h>
#include <TM/GCF_Task.h>
#include <TM/Socket/GCF_TCPPort.h>


class GCFEvent;
class GCFPortInterface; 

class GPAController : public GCFTask
{
	public:
		GPAController();
		virtual ~GPAController();
  
    void loadAPCTest();

	private: // GPAUsecountManager call back methods
    friend class GPAUsecountManager;
		void propertiesCreated(list<string>& propList);
    void propertiesDeleted(list<string>& propList);
    void allPropertiesDeletedByScope();
    void allPropertiesDeleted();
	
	private: // helper methods
    friend class GPAScopeManager;
    void doNextRequest();
    void loadAPC(char* actionData);
    void apcLoaded(TPAResult result);
    void unloadAPC(char* actionData);
    void apcUnloaded(TPAResult result);
    void reloadAPC(char* actionData);
    void unregisterScope(char* pScopeData);
    void propertiesLinked(char* pResponseData);
    void propertiesUnlinked(char* pResponseData);
    void sendAPCActionResponse(GCFEvent& e);
    void sendUnLinkActionResponse(GCFEvent& e);
    void unpackAPCActionData(char* pActionData);
    
	private: // state methods
		int initial(GCFEvent& e, GCFPortInterface& p);
		int connected(GCFEvent& e, GCFPortInterface& p);

	private: // data members
		GPAUsecountManager 	_usecountManager;
		GPARequestManager 	_requestManager;
		GPAScopeManager 		_scopeManager;

    list<GCFPortInterface*> _pmlPorts;		
		GCFTCPPort					_pmlPortProvider;
    
  private: // admin. data members
    string _curApcName;
    string _curScope;
    GCFPortInterface* _curRequestPort;
    bool _isBusy;
    unsigned int _counter;
    GPAAPC _apc;
};

#endif
