//#  MACScheduler.h: Interface between MAC and SAS.
//#
//#  Copyright (C) 2002-2004
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

#ifndef MACScheduler_H
#define MACScheduler_H

//# Includes
#include <boost/shared_ptr.hpp>

//# GCF Includes
#include <GCF/PAL/GCF_MyPropertySet.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Event.h>

//# local includes
#include "APL/APLCommon/PropertySetAnswerHandlerInterface.h"
#include "APL/APLCommon/PropertySetAnswer.h"
#include "APL/APLCommon/APLCommonExceptions.h"
#include "APL/APLCommon/LogicalDevice_Protocol.ph"
#include "APL/APLCommon/StartDaemon_Protocol.ph"

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/LofarLogger.h>

//# ACC Includes
#include <OTDB/OTDBconnection.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/OTDBnode.h>
#include <APS/ParameterSet.h>

// forward declaration

namespace LOFAR {
	namespace MCU {

using	GCF::TM::GCFTCPPort;
using	GCF::TM::GCFEvent;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTask;


class MACScheduler : public GCFTask,
							APLCommon::PropertySetAnswerHandlerInterface
{
public:
	MACScheduler();
	~MACScheduler();

   	// PropertySetAnswerHandlerInterface method
   	virtual void handlePropertySetAnswer(GCFEvent& answer);

	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult initial_state (GCFEvent& e, 
									 GCFPortInterface& p);
	
	// In this state the last-registered state is compared with the current
	// database-state and an appropriate recovery is made for each observation.
   	GCFEvent::TResult recover_state (GCFEvent& e, 
									 GCFPortInterface& p);

	// Normal control mode. Watching the OTDB and controlling the observations.
   	GCFEvent::TResult active_state  (GCFEvent& e, 
									 GCFPortInterface& p);

private:
	// avoid copying
	MACScheduler(const MACScheduler&);
   	MACScheduler& operator=(const MACScheduler&);

   	void _connectedHandler(GCFPortInterface& port);
   	void _disconnectedHandler(GCFPortInterface& port);
   	void _doOTDBcheck();
   	boost::shared_ptr<ACC::APS::ParameterSet> 
		 readObservationParameters (OTDB::treeIDType	ObsTreeID);

   	typedef boost::shared_ptr<GCF::PAL::GCFMyPropertySet> GCFMyPropertySetPtr;

   	APLCommon::PropertySetAnswer  itsPropertySetAnswer;
   	GCFMyPropertySetPtr           itsPropertySet;

#if 0
   	typedef GCFTCPPort  		TRemotePort;

   	typedef boost::shared_ptr<GCFTCPPort>  TTCPPortPtr;
   	typedef boost::shared_ptr<TRemotePort>  		TRemotePortPtr;
   	typedef vector<TTCPPortPtr>             		TTCPPortVector;
   	typedef vector<TRemotePortPtr>          		TRemotePortVector;
   	typedef map<string,TRemotePortPtr>      		TStringRemotePortMap;
   	typedef map<string,TTCPPortPtr>         		TStringTCPportMap;
      
   	bool 	_isServerPort	   (const GCFPortInterface& server, 
							    const GCFPortInterface& port) const;
   	bool 	_isVISDclientPort  (const GCFPortInterface& port, 
							  	string& visd) const;
   	bool 	_isVIclientPort    (const GCFPortInterface& port) const;
   	string 	_getVInameFromPort (const GCFPortInterface& port) const;
   	string 	_getShareLocation  () const;

   	void 	createChildsSections(OTDB::TreeMaintenance& tm, 
								int32 treeID, 
								OTDB::nodeIDType topItem, 
								const string& nodeName, 
								boost::shared_ptr<ACC::APS::ParameterSet> ps);
      
   	void 	_schedule		   (const string& VIrootID, 
								GCFPortInterface* port=0);
   	void 	_updateSchedule	   (const string& VIrootID, 
								GCFPortInterface* port=0);
   	void 	_cancelSchedule	   (const string& VIrootID, 
								GCFPortInterface* port=0);
      
   	TStringRemotePortMap	m_VISDclientPorts;    // connected VI StartD clients
   	string					m_VIparentPortName;
   	TRemotePort				m_VIparentPort;       // parent for VI's

   	// the vector and map both contain the child ports. The vector is used
   	// to cache the port at the moment of the accept. However, at that moment, 
   	// the parent does not yet know the ID of that child. The child sends its
   	// ID in the CONNECT event and when that message is received, the port and ID
   	// are stored in the TPortMap. The map is used in all communication with the
   	// childs.
   	TRemotePortVector		m_VIclientPorts;      // created VI's
   	TStringRemotePortMap	m_connectedVIclientPorts; // maps node ID's to ports
#endif

	// Administration of the ObservationControllers
	typedef struct {
		OTDB::treeIDType	treeID;		// tree in the OTDB
		GCFTCPPort*			port;		// TCP connection with controller
		uint16				state;		// state the controller has
	} ObsCntlr_t;

	// Map with all active ObservationControllers.
	map<GCFTCPPort*, ObsCntlr_t>	itsObsCntlrMap;
	vector<GCFTCPPort*>				itsObsCntlrPorts;

	// Ports for StartDaemon and ObservationControllers.
   	GCFTCPPort*				itsSDclientPort;		// connection to StartDaemon
   	GCFTCPPort*				itsLDserverPort;		// listener for ObsControllers

	// Second timer used for internal timing.
	uint32					itsSecondTimer;			// 1 second hardbeat
	uint32					itsSDretryTimer;		// for reconnecting to SD

	// Scheduling settings
	uint32					itsQueuePeriod;			// period between qeueuing and start
	uint32					itsClaimPeriod;			// period between claiming and start
      
	// OTDB related variables.
   	OTDB::OTDBconnection*	itsOTDBconnection;		// connection to the database
	uint32					itsOTDBpollInterval;	// itv between OTDB polls
	int32					itsNextOTDBpolltime;	// when next OTDB poll is scheduled

};

  };//MCU
};//LOFAR
#endif
