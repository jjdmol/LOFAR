//#  MISDaemon.cc: 
//#
//#  Copyright (C) 2002-2008
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>

#include <Common/ParameterSet.h>
#include <MACIO/MACServiceInfo.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSresult.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include "MISSession.h"
#include "MIS_Protocol.ph"
//#include "MISDefines.h"
#include "MISDaemon.h"
//MAXMOD add for antenna coords
//#include <APL/CAL_Protocol/SubArray.h>
#include <APL/CAL_Protocol/CAL_Protocol.ph>

namespace LOFAR {
	using namespace MACIO;
	using namespace GCF::TM;
	namespace AMI {

//
// MISDaemon()
//
MISDaemon::MISDaemon() :
	GCFTask((State)&MISDaemon::initial, "MisDaemon")
{
	// register the protocol for debugging purposes
	registerProtocol(MIS_PROTOCOL, MIS_PROTOCOL_STRINGS);
	registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_STRINGS);

	// initialize the ports
	itsListener = new GCFTCPPort(*this, "listener", GCFPortInterface::MSPP, MIS_PROTOCOL);
	ASSERTSTR(itsListener, "Can't allocate a listener port");
	itsListener->setPortNumber(SHM_INFOSERVER_PORT);

//	itsDPService = new DPservice(this);
//	ASSERTSTR(itsDPservice, "Can't allocate DataPoint service");

	// try to add the RemoteStation parameters to my parameterset.
	try {
		ConfigLocator	aCL;
		globalParameterSet()->adoptFile(aCL.locate("RemoteStation.conf"),"");
	}
	catch (Exception& e) {
	        LOG_WARN_STR("Error: failed to load configuration files: " << e.text());
	}  
	//MAXMOD
	//Put the AntennaArrays.conf read in here
	try {
	  ConfigLocator  aCL;
	  // Load antenna arrays
	  m_arrays.getAll(aCL.locate("AntennaArrays.conf"));
	  //vector<string> ArrayNames = m_arrays.getNameList();
	  //vector<string>::iterator	iter = ArrayNames.begin();
	  //vector<string>::iterator	end  = ArrayNames.end();
	  //LOG_INFO(formatString("MISDaemon loaded the following Antenna Arrays:"));
	  //while (iter != end) {
	    //const CAL::AntennaArray * somearray = m_arrays.getByName(*iter);
	    //cout << "iter  :" << *iter << endl;
	    //cout << "somearray.getName() :" << somearray->getName() << endl;
	    //iter++;
	  //}
	}
	catch (Exception& e) {
	  LOG_WARN_STR("Error: failed to load configuration files: " << e.text());
	}
}

//
// ~MISDaemon()
//
MISDaemon::~MISDaemon()
{
//	if (itsDPservice) {
//		delete itsDPService;
//	}
	if (itsListener) {
		delete itsListener;
	}
}


//
// initial(event,port)
//
GCFEvent::TResult MISDaemon::initial(GCFEvent& e, GCFPortInterface& p)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
	case F_TIMER:
		if (!itsListener->isConnected()) {
			itsListener->open();
		}
	break;

	case F_CONNECTED:
		TRAN(MISDaemon::accepting);
	break;

	case F_DISCONNECTED:
		p.setTimer(5.0); // try again after 5 second
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
	break;
	}

	return status;
}

//
// accepting(event.port)
//
GCFEvent::TResult MISDaemon::accepting(GCFEvent& e, GCFPortInterface& p)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	static unsigned long garbageTimerID = 0;
//	static unsigned long rereadPolicyTimerID = 0;
//	static bool hasPVSS = false; 

	switch (e.signal) {
	case F_ENTRY: {
		garbageTimerID      = itsListener->setTimer(5.0, 5.0); 
//		rereadPolicyTimerID = itsListener->setTimer(60.0, 60.0); 
		break;
	}

	case F_DISCONNECTED:
		DBGFAILWHEN(itsListener == &p && "MISD port provider may not be disconnected."); 
		break;

	case F_TIMER: {
		GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(e);

		if (timerEvent.id == garbageTimerID) {
			// cleanup the garbage of closed ports to master clients
			MISSession* pClient;
			for (TSessions::iterator iter = _sessionsGarbage.begin(); iter != _sessionsGarbage.end(); ++iter) {
				pClient = *iter;
				delete pClient;
			}
			_sessionsGarbage.clear();
		}
/*		else if (timerEvent.id == rereadPolicyTimerID) {
			_policyHandler.rereadPolicyFile();
		}      
*/
		break;
	}  

	case F_CLOSED:
		DBGFAILWHEN(&p == itsListener);
		break;

	case F_CONNECTED:
		DBGFAILWHEN(&p == itsListener);
		break;

	case F_ACCEPT_REQ: {
		LOG_INFO("New MIS client accepted!");
		MISSession* miss = new MISSession(*this);
		miss->start();
#if 0
		if (!hasPVSS) {
			// now we have PVSS connection
			hasPVSS = true;        
			// the GCFPVSSInfo::getOwnManNum() method only returns a valid
			// man number if a PVSS connections has been established
			// is will be automatically done by the MISSession
			SKIP_UPDATES_FROM(GCFPVSSInfo::getOwnManNum());
		}
#endif
		break;
	}

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// clientClosed(client)
//
void MISDaemon::clientClosed(MISSession& client)
{
	_sessionsGarbage.push_back(&client);  
}

 } // namespace AMI
} // namespace LOFAR
