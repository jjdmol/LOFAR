//#  ServiceBrokerTask.h: singleton class; bridge between controller application 
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

#ifndef GTM_SERVICEBROKER_H
#define GTM_SERVICEBROKER_H

#include <Common/lofar_map.h>
#include <Common/lofar_list.h>
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Task.h>
#include "GTM_SBTCPPort.h"
#include <GCF/TM/GCF_Handler.h>

namespace LOFAR {
 using MACIO::GCFEvent;
 namespace GCF {  
  namespace SB {

/**
*/

class GTMSBHandler;

class ServiceBrokerTask : public TM::GCFTask
{
public:
    ~ServiceBrokerTask ();
    static ServiceBrokerTask* instance(bool temporary = false);
    static void release();

	// member functions
    void registerService  (TM::GCFTCPPort&	servicePort);
    void unregisterService(TM::GCFTCPPort&	servicePort);
    void getServiceinfo   (TM::GCFTCPPort&	clientPort, 
						   const string& 	remoteServiceName,
						   const string&	hostname);
    void deletePort		  (TM::GCFTCPPort&	port);
  
private:
    friend class GTMSBHandler;
    ServiceBrokerTask ();

	// state methods
    GCFEvent::TResult operational (GCFEvent& e, TM::GCFPortInterface& p);
        
	// helper structures and classes
    typedef struct action_t {
		uint16				seqnr;
		uint16 				type;
		TM::GCFTCPPort*		pPort;
		string 				servicename;
		string				hostname;
		time_t				timestamp;
		action_t& operator= (const action_t& other) {        
			if (this != &other) {
				seqnr		= other.seqnr;
				type		= other.type;
				pPort		= other.pPort;
				servicename = other.servicename;          
				hostname	= other.hostname;
				timestamp	= other.timestamp;
			}
			return (*this);
		}      
    } Action;
	typedef struct service_t {
		string		servicename;
		uint16		portNr;
		service_t(string s, uint16 p) : servicename(s), portNr(p) {};
		service_t() 				  : servicename(),  portNr(0) {};
	} KnownService;
    typedef list<Action>								actionList_t;
	typedef	map<string /*hostname*/, GTMSBTCPPort*>		brokerMap_t;
	typedef	map<GCFTCPPort*,         KnownService>		serviceMap_t;
	typedef actionList_t::iterator			ALiter;
	typedef brokerMap_t::iterator			BMiter;
	typedef serviceMap_t::iterator			SMiter;

	// helper methods
	void	_deleteService     (GCFTCPPort&			aPort);
    uint16	_registerAction    (Action 				action);
	void	_doActionList	   (const string&		hostname);
	ALiter	_findAction		   (uint16				seqnr);
	BMiter	_getBroker		   (const string&		hostname);
	void	_reconnectBrokers  ();
	void	_checkActionList   (const string&		hostname);
	void	_reRegisterServices(GCFPortInterface*	brokerPort);
	string	_actionName		   (uint16				actionType) const;
	void	_logResult		   (uint16				result, 
							    const string& 		servicename, 
							    const string& 		hostname) const;

	// data members        
	uint16				itsSeqnr;			// for actionlist.
	int32				itsMaxResponse;		// never let client wait longer.
	GCFTimerPort		itsTimerPort;		// for reconnecting to brokers
	brokerMap_t			itsBrokerMap;		// connections to all Brokers
    actionList_t 		itsActionList;    	// deferred actions
	serviceMap_t		itsServiceMap;		// services I registered

};

class GTMSBHandler : public TM::GCFHandler
{
public:
    ~GTMSBHandler() { _pInstance = 0; }
    void workProc() {}
    void stop () {}
    
private:
    friend class ServiceBrokerTask;
    GTMSBHandler();

    static GTMSBHandler* _pInstance;
    ServiceBrokerTask _controller;
};
  } // namespace SB
 } // namespace GCF
} // namespace LOFAR
#endif
