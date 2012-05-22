//#  GTM_SBTCPPort.cc: connection to a remote process
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include "GTM_SBTCPPort.h"
#include <GTM_Defines.h>
//#include "GSB_Defines.h"
#include <MACIO/MACServiceInfo.h>
#include <PortImpl/GTM_TCPServerSocket.h>
#include <Common/ParameterSet.h>

namespace LOFAR {
 namespace GCF {
  using namespace TM;
  namespace SB {

//
// GTMSBTCPPort(task, name, type, protocol, raw)
//
GTMSBTCPPort::GTMSBTCPPort(GCFTask& 	 task, 
						   const string& name, 
						   TPortType 	 type, 
						   int 			 protocol, 
						   bool 		 transportRawData) 
  : GCFTCPPort(task, name, type, protocol, transportRawData)
{
}

//
// GTMSBTCPPort()
//
GTMSBTCPPort::GTMSBTCPPort()
    : GCFTCPPort()
{
}

//
// ~GTMSBTCPPort()
//
GTMSBTCPPort::~GTMSBTCPPort()
{
	if (_pSocket) {
		delete _pSocket;  
		_pSocket = 0;  
	}
}


//
// open()
//
bool GTMSBTCPPort::open()
{
	if (isConnected()) {
		LOG_INFO(formatString ("Port %s already open.", makeServiceName().c_str()));
		return (false);
	}

	if (!_pSocket) {
		_pSocket = new GTMTCPSocket(*this);
		ASSERTSTR(_pSocket, "Could not create GTMTCPSocket for SBtask");
		_pSocket->setBlocking(false);
	}

	uint32	sbPortNumber(MAC_SERVICEBROKER_PORT);

	setState(S_CONNECTING);
	if (!_pSocket->open(sbPortNumber)) { 
		_handleDisconnect();
		return (false);
	}

	switch (_pSocket->connect(sbPortNumber, getHostName())) {
	case -1: _handleDisconnect();  break; 
	case 0:		// in progress
		LOG_INFO_STR("GTMSBTCPPort:connect(" << getHostName() << ") still in progress");
		if (!itsConnectTimer) {
//			itsConnectTimer = setTimer(*this, (uint64)(1000000.0), (uint64)(1000000.0), &itsConnectTimer);
			itsConnectTimer = setTimer(1.0, 1.0, &itsConnectTimer);
		}
		break;
	case 1: 
		_handleConnect();
		return (true);
	}
	return (false);
}

void GTMSBTCPPort::_handleDisconnect()
{
	setState(S_DISCONNECTING);
	cancelTimer(itsConnectTimer);
	itsConnectTimer = 0;
	_pSocket->setBlocking(true);
	schedule_disconnected();
}
	
void GTMSBTCPPort::_handleConnect()
{
	cancelTimer(itsConnectTimer);
	itsConnectTimer = 0;
	_pSocket->setBlocking(true);
	schedule_connected();
}

//
// dispatch(event)
//
GCFEvent::TResult GTMSBTCPPort::dispatch(GCFEvent&	event) {
	if (event.signal == F_TIMER) {
		GCFTimerEvent	*TEptr = static_cast<GCFTimerEvent*>(&event);
		if (TEptr->arg == &itsConnectTimer) {
			open();
			return (GCFEvent::HANDLED);
		}
	}

	return (GCFTCPPort::dispatch(event));
}

	
  } // namespace SB
 } // namespace GCF
} // namespace LOFAR
