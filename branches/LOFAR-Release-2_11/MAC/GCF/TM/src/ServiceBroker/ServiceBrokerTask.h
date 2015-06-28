//#  ServiceBrokerTask.h: singleton class; bridge between controller application 
//#                    and Property Agent
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
#include <GCF/TM/GCF_Handler.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_TimerPort.h>
#include "GTM_SBTCPPort.h"

namespace LOFAR {
  using MACIO::GCFEvent;
  namespace GCF {
	using TM::GCFPortInterface;
	using TM::GCFTCPPort;
	using TM::GCFTimerPort;
	using TM::GCFTask;
	using TM::GCFHandler;
	namespace SB {

/**
*/

class GTMSBHandler;

class ServiceBrokerTask : public GCFTask
{
public:
    ~ServiceBrokerTask ();
    static ServiceBrokerTask* instance(bool temporary = false);
    static void release();

	// member functions
    void registerService  (GCFTCPPort&	servicePort);
    void unregisterService(GCFTCPPort&	servicePort);
    void getServiceinfo   (GCFTCPPort&	clientPort, 
						   const string& 	remoteServiceName,
						   const string&	hostname);
    void deletePort		  (GCFTCPPort&	port);
  
private:
	static const int MAX_RECONNECT_RETRIES = 5;
    friend class GTMSBHandler;
    ServiceBrokerTask ();

	// state methods
    GCFEvent::TResult operational (GCFEvent& e, GCFPortInterface& p);
        
	// helper structures and classes
    typedef struct action_t {
		uint16			seqnr;
		uint16 			type;
		GCFTCPPort*		pPort;
		string 			servicename;
		string			hostname;
		time_t			timestamp;

		string print() const {
			stringstream	oss;
			oss << "Action[" << seqnr << ": " << type << ", " << servicename << "@" << hostname << "]";
			return (oss.str());
		}
    } Action;
	typedef struct service_t {
		string		servicename;
		uint16		portNr;
		service_t(string s, uint16 p) : servicename(s), portNr(p) {};
		service_t() 				  : servicename(),  portNr(0) {};
	} KnownService;
	typedef struct broker_info_t {
		GTMSBTCPPort*		port;
		int					nRetries;
		broker_info_t(GTMSBTCPPort* p, int n) : port(p), nRetries(n) {};
		broker_info_t() : port(0), nRetries(0) {};
	} BrokerInfo;
    typedef list<Action>								actionList_t;	
	typedef	map<string /*hostname*/, BrokerInfo>		brokerMap_t;
	typedef	map<GCFTCPPort*,         KnownService>		serviceMap_t;
	typedef actionList_t::iterator			ALiter;
	typedef brokerMap_t::iterator			BMiter;
	typedef serviceMap_t::iterator			SMiter;

	// helper methods
	void	_deleteService     (GCFTCPPort&			aPort);
    uint16	_registerAction    (Action 				action);
	void	_doActionList	   (const string&		hostname);
	void	_printActionList   ();
	ALiter	_findAction		   (uint16				seqnr);
	BMiter	_getBroker		   (const string&		hostname);
	void	_lostBroker		   (const string& 		hostname);
	void	_reconnectBrokers  ();
	void	_checkActionList   (const string&		hostname);
	void	_reRegisterServices(GCFPortInterface*	brokerPort);
	string	_actionName		   (uint16				actionType) const;
	void	_logResult		   (uint16				result, 
							    const string& 		servicename, 
							    const string& 		hostname) const;

	// data members        
	uint16				itsSeqnr;			// for actionlist.
	int32				itsMaxConnectTime;	// never let client wait longer for a connection.
	int32				itsMaxResponseTime;	// never let client wait longer for an answer.
	GCFTimerPort		itsTimerPort;		// for reconnecting to brokers
	brokerMap_t			itsBrokerMap;		// connections to all Brokers
    actionList_t 		itsActionList;    	// deferred actions
	serviceMap_t		itsServiceMap;		// services I registered

};

class GTMSBHandler : public GCFHandler
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
