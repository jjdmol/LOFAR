//#  GCF_PVSSPort.cc: connection to a remote process
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

#define LOFARLOGGER_SUBPACKAGE "SAL"

#include <GCF/PAL/GCF_PVSSPort.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GSA_Defines.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Protocols.h>

#include <GSA_PortService.h>

namespace LOFAR 
{
 namespace GCF 
 {
using namespace Common;   
using namespace TM;
  namespace PAL
  {
static GCFEvent disconnectedEvent(F_DISCONNECTED);
static GCFEvent connectedEvent   (F_CONNECTED);
static GCFEvent closedEvent      (F_CLOSED);
GCFPVSSPort::TPVSSPortNrs GCFPVSSPort::_pvssPortNrs;

//
// GCFPVSSPort (task, name , type, protocol, raw)
//
GCFPVSSPort::GCFPVSSPort(GCFTask& 		task, 
						 const string&	name, 
						 TPortType 		type, 
						 int 			protocol, 
						 bool 			transportRawData) 
  : GCFRawPort(task, name, type, protocol, transportRawData),
    _pPortService(0),
    _acceptedPort(false)
{
}

//
// GCFPVSSPort()
//
GCFPVSSPort::GCFPVSSPort()
    : GCFRawPort(),
    _pPortService(0),
    _acceptedPort(false)
{
}

//
// ~GCFPVSSPort()
//
GCFPVSSPort::~GCFPVSSPort()
{
	if (_pPortService) {
		if (_acceptedPort) {      
			_pPortService->unregisterPort(_remotePortId);
		}
		else {
			releasePortNr(_portId.getValue());
			_pPortService->stop();
			delete _pPortService;
			_pPortService = 0;    
		}
	}
}

//
// open()
//
bool GCFPVSSPort::open()
{
	if (getState() != S_DISCONNECTED) {
		LOG_ERROR(formatString ( "ERROR: Port %s already open.", _name.c_str()));
		return false;
	}

	if (!_pPortService) {
		if (isSlave()) {
			LOG_ERROR(formatString ( "ERROR: Port %s not initialised.", _name.c_str()));
			return false;
		}

		_pPortService = new GSAPortService(*this);
		_pPortService->setConverter(*_pConverter);
		unsigned int pvssPortNr = claimPortNr();
		if (_type == SAP) {
			_portAddr.setValue(formatString(
								"%s:__gcfportAPI%d_%d_%s", 
								GCFPVSSInfo::getLocalSystemName().c_str(),
								GCFPVSSInfo::getOwnManNum(), 
								pvssPortNr,
								getRealName().c_str()));
		}
		else {
			_portAddr.setValue(GCFPVSSInfo::getLocalSystemName() + ":__gcfportAPI_" + getRealName());
		}

		_portId.setValue(formatString(
							"%d:Api:%d:%d:",
							GCFPVSSInfo::getLocalSystemId(),
							GCFPVSSInfo::getOwnManNum(),
							pvssPortNr));
}

	setState(S_CONNECTING);
	_pPortService->start();
	return true;
}

//
// serviceStarted(successful)
//
void GCFPVSSPort::serviceStarted(bool successfull)
{
	if (successfull)
		schedule_connected();
	else
		schedule_disconnected();
}

//
// setService(service)
//
void GCFPVSSPort::setService(GSAPortService& service)
{ 
	_pPortService = &service; 
	_acceptedPort = true; 
	_portAddr.setValue(service.getPort().getPortAddr());
	_portId.setValue(service.getPort().getPortID());
	_remotePortId = _pPortService->getCurPortId();
}

//
// send (event)
//
ssize_t GCFPVSSPort::send(GCFEvent& e)
{
	ssize_t written = 0;

	ASSERT(_pPortService);

	if (MSPP == getType())  
		return -1; // no messages can be send by this type of port

	unsigned int packsize;
	void* buf = e.pack(packsize);

	unsigned int newBufSize = packsize + 1 + _portId.getSize() + _portAddr.getSize();
	char* newBuf = new char[newBufSize];
	newBuf[0] = 'm'; // just a message
	unsigned int bytesPacked = 1;
	bytesPacked += _portId.pack(newBuf + bytesPacked);
	bytesPacked += _portAddr.pack(newBuf + bytesPacked);
	memcpy(newBuf + bytesPacked, buf, packsize);

	LOG_TRACE_FLOW(formatString("Sending event '%s' for task '%s' on port '%s'",
							getTask()->evtstr(e),
							getTask()->getName().c_str(), 
							getRealName().c_str()));

	if ((written = _pPortService->send(newBuf, newBufSize, _destDpName)) != (ssize_t) newBufSize) {  
		setState(S_DISCONNECTING);     
		schedule_disconnected();

		written = -1;
	}

	delete [] newBuf;

	return written;
}

//
// recv(buf, count)
//
ssize_t GCFPVSSPort::recv(void* buf, size_t count)
{
	ASSERT(_pPortService);
	return _pPortService->recv(buf, count);
}

//
// close()
//
bool GCFPVSSPort::close()
{
	setState(S_CLOSING);  
	ASSERT(_pPortService);

	if (_acceptedPort) {
		_pPortService->unregisterPort(_remotePortId);
	}
	else {
		releasePortNr(_portId.getValue());
		_pPortService->stop();    
	}
	schedule_close();
	// return success when port is still connected
	// scheduled close will only occur later
	return isConnected();
}

//
// setDestAddr(destDpName)
//
void GCFPVSSPort::setDestAddr(const string& destDpName)
{
	_destDpName.clear();
	_destDpName += destDpName;
}

//
// accept(port)
//
bool GCFPVSSPort::accept(GCFPVSSPort& newPort)
{
	if (MSPP == getType() && SPP == newPort.getType()) {
		ASSERT(_pPortService);
		newPort.setService(*_pPortService);
		_pPortService->registerPort(newPort);
		newPort.setState(S_CONNECTING);    
		return true;
	}

	return false;
}

//
// claimPortNr()
//
unsigned int GCFPVSSPort::claimPortNr()
{
	unsigned int nr(0);

	TPVSSPortNrs::iterator iter;
	do {
		nr++;
		iter = _pvssPortNrs.find(nr);
	}
	while (iter != _pvssPortNrs.end()); 

	return nr; 
}

//
// releasePortNr(portID)
//
void GCFPVSSPort::releasePortNr(const string& portId)
{
	string::size_type pos = portId.rfind(':');
	if (pos > 0 && pos < string::npos) {
		unsigned int portNr = atoi(portId.c_str() + pos + 1);
		_pvssPortNrs.erase(portNr);
	}
}

//
// setConverter(converter)
//
void GCFPVSSPort::setConverter(GCFPVSSUIMConverter& converter) 
{ 
	_pConverter = &converter; 
	if (_pPortService) 
		_pPortService->setConverter(converter);
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
