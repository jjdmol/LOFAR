//#  NenuFarIO.cc: Implements all IO with the NenuFar server.
//#
//#  Copyright (C) 2002-2014
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
//#  $Id: $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <Common/ParameterSet.h>
#include <GCF/TM/GCF_Protocols.h>
#include "NenuFarMsg.h"
#include "NenuFarAdmin.h"
#include "NenuFarIO.h"

const int	MAX_DATA_LEN	= 10000;	// just some sane number

namespace LOFAR {
  using namespace GCF::TM;
  namespace BS {

//
// NenuFarIO (NenuFarAdmin*)
//
NenuFarIO::NenuFarIO(NenuFarAdmin*	nnfAdmin) :
	GCFTask		((State)&NenuFarIO::connect2server,"NenuFarIO"),
	itsBeams	(nnfAdmin)
{
	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "NenuFarIOtimer");

	// prepare TCP port to NCU.
	itsNCUport = new GCFTCPPort (*this, "NCU_IO_port", GCFPortInterface::SAP, 0, true);	// raw data
	ASSERTSTR(itsNCUport, "Cannot allocate TCPport to NenuFar system");

	// enable interface the BeamServerTask uses.
	itsBeams->activateAdmin();

	LOG_INFO("NenuFarIO task succesfully initialized");
}

//
// ~NenuFarIO()
//
NenuFarIO::~NenuFarIO()
{
	if (itsNCUport) {
		itsNCUport->close();
		delete itsNCUport;
	}
	
	if (itsTimerPort)
		delete itsTimerPort;
}


//
// connect2server(event, port)
//
// Setup connection with NCU
//
GCFEvent::TResult NenuFarIO::connect2server(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("connect2server:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
    case F_ENTRY:
		// read in settings...
		try {
			itsNCUhostname	  = globalParameterSet()->getString("BeamServer.NenuFar.hostname");
			itsNCUportnr	  = globalParameterSet()->getInt   ("BeamServer.NenuFar.portnr");
			itsReconInterval  = globalParameterSet()->getDouble("BeamServer.NenuFar.connectInterval", 10.0);
			itsIOtimeout 	  = globalParameterSet()->getDouble("BeamServer.NenuFar.respondTime", 3.0);
			LOG_INFO_STR("Expecting NenuFar system is listening at " << itsNCUhostname << ":" << itsNCUportnr);
		}
		catch (Exception&	ex) {
			cerr << ex << endl;
			abort();
		}
		break;

	case F_INIT: {
		// Try to establish a connection with the NenuFar Control Unit...
		itsNCUport->setHostName(itsNCUhostname);
		itsNCUport->setPortNumber(itsNCUportnr);
		// try open the connection, report failure after 1 minute
		itsNCUport->autoOpen(-1, 60, 10.0);	// nrRetries, maxWaitTime, reconnectinterval
	} break;

	case F_CONNECTED: {
		LOG_INFO_STR("Connected to NenuFar system");
		TRAN(NenuFarIO::monitorAdmin);
	} break;

	case F_DISCONNECTED: {
		LOG_INFO_STR("Still not connected to the NenuFar system");
		// keep trying, report failure once every 5 minutes.
		itsNCUport->autoOpen(-1, 300, 10.0);	// nrRetries, maxWaitTime, reconnectinterval
	} break;

	case F_QUIT:
		TRAN (NenuFarIO::finish_state);
		break;

	case F_EXIT:
		break;

	default:
		LOG_DEBUG_STR ("connect2server, DEFAULT");
		break;
	}    
	
	return (GCFEvent::HANDLED);
}


//
// monitorAdmin(event, port)
//
// monitor administration report deltas to NenuFar
//
GCFEvent::TResult NenuFarIO::monitorAdmin(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("monitorAdmin:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {	
		itsTimerPort->setTimer(0.0);	// evaluate immediately
	} break;

	case F_TIMER: {
		// evaluate the changes...
		if (itsBeams->allBeamsAborted()) {
			ParameterSet	ps;
			NenuFarMsg	theMsg(0x0100, ABORT_ALL_BEAMS_MSG, ps);
			LOG_INFO_STR("Sending: ABORT ALL BEAMS");
			itsNCUport->send(theMsg.data(), theMsg.size());
		}
		else {
			time_t		now(time(0L));
			itsCurrentBeam = itsBeams->firstCommand(now);
			if (!itsCurrentBeam.name().empty()) {		// still work to do?
				TRAN(NenuFarIO::tranceiveAdminChange);
				return (GCFEvent::HANDLED);
			}
		}
		itsTimerPort->setTimer(1.0);		// re-evaluate every second.
	} break;

	case F_DISCONNECTED: {
		LOG_INFO_STR("Lost connection with NenuFar, trying to reconnect");
		TRAN(NenuFarIO::connect2server);
	} break;

	case F_EXIT: {
		itsTimerPort->cancelAllTimers();
	} break;

	case F_QUIT: {
		TRAN (NenuFarIO::finish_state);
	} break;

	default:
		LOG_DEBUG_STR ("monitorAdmin, DEFAULT");
		break;
	}    
	
	return (GCFEvent::HANDLED);
}

//
// tranceiveAdminChange(event, port)
//
// Send the most recent change to the NCU
//
GCFEvent::TResult NenuFarIO::tranceiveAdminChange(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("transceiveAdminChange:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {	
		// Check msgtype before sending the message
		NenuFarMsg*		theMsg;
		switch (itsCurrentBeam.beamState()) {
		case	NenuFarAdmin::BeamInfo::BS_NEW:	{ 
			ParameterSet	ps = itsCurrentBeam.asParset();
			theMsg = new NenuFarMsg(0x0100, NEW_BEAM_MSG, ps);
		} break; 
		case	NenuFarAdmin::BeamInfo::BS_ENDED: { 
			ParameterSet	ps = itsCurrentBeam.nameAsKV();
			theMsg = new NenuFarMsg(0x0100, STOP_BEAM_MSG, ps);
		} break;
		case	NenuFarAdmin::BeamInfo::BS_ABORT: { 
			ParameterSet	ps = itsCurrentBeam.nameAsKV();
			theMsg = new NenuFarMsg(0x0100, ABORT_BEAM_MSG, ps);
		} break;
		default:
			LOG_ERROR_STR("Unsupported beam state (" << NenuFarAdmin::BeamInfo::stateName(itsCurrentBeam.beamState()) << 
						  "} for beam " << itsCurrentBeam.name());
			TRAN(NenuFarIO::monitorAdmin);
			return (GCFEvent::HANDLED);
		}
		// Type is ok, safe to send it now
		LOG_INFO_STR("Sending: " << NenuFarAdmin::BeamInfo::stateName(itsCurrentBeam.beamState()) << 
					 " for " << itsCurrentBeam.name());
		itsNCUport->send(theMsg->data(), theMsg->size());
		delete theMsg;

		// now wait for an answer
		itsTimerPort->setTimer(itsIOtimeout);
		itsIOerror = false;
	} break;

	case F_DATAIN: {
		// first read header that containts the size of the data parts
		char	hdrbuf[HDR_SIZE];
		size_t	btsRead = itsNCUport->recv(&hdrbuf, HDR_SIZE);
		if (btsRead != HDR_SIZE) {
			LOG_ERROR_STR("Received NenuFar answer that is too short. (" << btsRead << ")");
			itsIOerror = true;
			TRAN(NenuFarIO::monitorAdmin);
			return (GCFEvent::HANDLED);
		}
		hdrbuf[HDR_SIZE-1] = '\0';

		// sanity check on datalen
		int	dataLen = atoi(hdrbuf);
		if (dataLen < 0 || dataLen > MAX_DATA_LEN) {
			LOG_ERROR_STR("NenuFar header contains illegal datalen. (" << dataLen << ")");
			itsIOerror = true;
			TRAN(NenuFarIO::monitorAdmin);
			return (GCFEvent::HANDLED);
		}
		
		// no weird numbers received, continue with the data part
		int		remainingBytes = dataLen;
		int		bufsize = HDR_SIZE + dataLen;
		char	databuf[bufsize];
		int		offset(HDR_SIZE);
		bcopy(hdrbuf, databuf, HDR_SIZE);
		while (remainingBytes > 0) {
			btsRead = itsNCUport->recv(&databuf[offset], remainingBytes);
			if (btsRead <= 0) {
				LOG_ERROR_STR("Read error on NenuFar socket: " << btsRead);
				itsIOerror = true;
				TRAN(NenuFarIO::monitorAdmin);
				return (GCFEvent::HANDLED);
			}
			offset 		   += btsRead;
			remainingBytes -= btsRead;
		}
		if (remainingBytes) {
			LOG_ERROR_STR("NenuFar message not received completely, missing " << remainingBytes << " bytes");
			itsIOerror = true;
			TRAN(NenuFarIO::monitorAdmin);
			return (GCFEvent::HANDLED);
		}
		databuf[bufsize-1] = '\0';

		// Yeah, message received completely, check contents.
		ParameterSet	answer = NenuFarMsg::unpack2parset(databuf, bufsize);
		LOG_DEBUG_STR("Received:::" << answer<<":::");
		if (!answer.isDefined("beamName")) {
			LOG_ERROR_STR("Answer for beam " << itsCurrentBeam.name() << " does not contain the 'beamName' field");
			itsIOerror = true;
			TRAN(NenuFarIO::monitorAdmin);
			return (GCFEvent::HANDLED);
		}
		try {
			switch (itsCurrentBeam.beamState()) {
			case	NenuFarAdmin::BeamInfo::BS_NEW:	{ 
				bool	accepted = answer.getBool("accepted");
				if (!accepted) {
					string 	reason = answer.getString("reason");
					LOG_ERROR_STR("Beam " << itsCurrentBeam.name() << " not accepted: " << reason);
				}
				else {
					itsBeams->setCommState(itsCurrentBeam.name(), itsCurrentBeam.beamState());
					LOG_INFO_STR("Beam " << itsCurrentBeam.name() << " accepted");
				}
			} break; 
			case	NenuFarAdmin::BeamInfo::BS_ENDED:
			case	NenuFarAdmin::BeamInfo::BS_ABORT: { 
				int		result = answer.getInt("result");
				if (result != 0) {
					LOG_ERROR_STR("Acknowledge on state " << NenuFarAdmin::BeamInfo::stateName(itsCurrentBeam.beamState()) << 
								  " of beam " << itsCurrentBeam.name() << " returned error " << result);
				}
				else {
					itsBeams->setCommState(itsCurrentBeam.name(), itsCurrentBeam.beamState());
					LOG_INFO_STR("Acknowledge on state " << NenuFarAdmin::BeamInfo::stateName(itsCurrentBeam.beamState()) << 
								 " of beam " << itsCurrentBeam.name() << " is OK");
				}
			} break;
			} // switch
		} catch (Exception& e) {
			LOG_ERROR_STR("Exception: " << e.text());
		}

		TRAN(NenuFarIO::monitorAdmin);
	} break;

	case F_TIMER: {
		// too bad, no answer from the NCU continue
		LOG_WARN_STR("Didn't receive an answer from NenuFar for state " << 
					  NenuFarAdmin::BeamInfo::stateName(itsCurrentBeam.beamState()) << " for beam " << itsCurrentBeam.name());
		itsBeams->setCommState(itsCurrentBeam.name(), NenuFarAdmin::BeamInfo::BS_ABORT);
//		itsIOerror = true;
		TRAN(NenuFarIO::monitorAdmin);
	} break;

	case F_DISCONNECTED: {
		LOG_INFO_STR("Lost connection with NenuFar, trying to reconnect");
		TRAN(NenuFarIO::connect2server);
	} break;

	case F_EXIT: {
		if (itsIOerror) {
			itsBeams->setCommState(itsCurrentBeam.name(), NenuFarAdmin::BeamInfo::BS_ABORT);
		}
		itsTimerPort->cancelAllTimers();
	} break;

	case F_QUIT: {
		TRAN (NenuFarIO::finish_state);
	} break;

	default:
		LOG_DEBUG_STR ("transceiveAdminChange, DEFAULT");
		break;
	}    
	
	return (GCFEvent::HANDLED);
}

GCFEvent::TResult NenuFarIO::finish_state (GCFEvent& event, GCFPortInterface& port)
{
	//TODO
	return (GCFEvent::HANDLED);
}


  } // namespace BS
} // namespace LOFAR
