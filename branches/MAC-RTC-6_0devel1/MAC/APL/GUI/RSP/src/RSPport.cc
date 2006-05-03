//#  RSPport.cc: one line description
//#
//#  Copyright (C) 2006
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/hexdump.h>
#include <GCF/TM/GCF_Event.h>
#include <RSP/RSPport.h>

namespace LOFAR {
  using RSP_Protocol::WGSettings;

  namespace RSP {

using	GCF::TM::GCFEvent;

static	char		receiveBuffer[24*4096];

//
// RSPport (host, port)
//
RSPport::RSPport(string	aHostname) :
	itsPort		   (24001),		// FOR THE TIME BEING!!!
	itsHost		   (aHostname),
	itsSocket	   (new Socket("RSPsocket", itsHost, toString(itsPort))),
	itsNrRCUs 	   (0),
	itsNrRSPboards (0),
	itsMaxRSPboards(0)
{
	itsSocket->connect(-1);			// try to connect, wait max 1 second
	itsSocket->setBlocking(true);	// no other tasks, do rest blocking

	RSPGetconfigEvent	getConfig;
	send(&getConfig);

	RSPGetconfigackEvent ack(receive());
	itsNrRCUs 		= ack.n_rcus;
	itsNrRSPboards  = ack.n_rspboards;
	itsMaxRSPboards = ack.max_rspboards;
}

//
// ~RSPport
//
RSPport::~RSPport()
{
	if (itsSocket) {
		itsSocket->shutdown();
		delete itsSocket;
	};
}

//
// send(Event*)
//
void RSPport::send(GCFEvent*	anEvent)
{
	// Serialize the message and write buffer to port
	uint32	packSize;
	void* buf = anEvent->pack(packSize);
	int32 btsWritten = itsSocket->write(buf, packSize);
	ASSERTSTR(btsWritten == (int32)packSize, 
			  "Only " << btsWritten << " of " << packSize << " bytes written");
}

//
// receive() : Event
//
GCFEvent&	RSPport::receive()
{
	// First read header if answer:
	// That is signal field + length field.
	GCFEvent*	header = (GCFEvent*) &receiveBuffer[0];
	int32		btsRead;
	btsRead = itsSocket->read((void*) &(header->signal), sizeof(header->signal));
	ASSERTSTR(btsRead == sizeof(header->signal), "Only " << btsRead << " of " 
						<< sizeof(header->signal) << " bytes of header read");
	btsRead = itsSocket->read((void*) &(header->length), sizeof(header->length));
	ASSERTSTR(btsRead == sizeof(header->length), "Only " << btsRead << " of " 
						<< sizeof(header->length) << " bytes of header read");

	LOG_DEBUG("Header received");

	// Is there a payload in the message? This should be the case!
	int32	remainingBytes = header->length;
	LOG_DEBUG_STR(remainingBytes << " bytes to get next");
	if (remainingBytes <= 0) {
		return(*header);
	}

	// read remainder
	btsRead = itsSocket->read(&receiveBuffer[sizeof(GCFEvent)], remainingBytes);
	ASSERTSTR(btsRead == remainingBytes,
		  "Only " << btsRead << " bytes of msg read: " << remainingBytes);

//	hexdump(receiveBuffer, sizeof(GCFEvent) + remainingBytes);

	// return Eventinformation
	return (*header);
}

//
// getStatus() : struct BoardStatus
//
vector<BoardStatus> RSPport::getBoardStatus(uint32	RCUmask)
{
	// Construct a query message
	RSPGetstatusEvent	question;
	question.timestamp	= RTC::Timestamp(0,0);
	question.rcumask 	= RCUmask;
	question.cache   	= false;

	send (&question);

	RSPGetstatusackEvent ack(receive());
	
	// Finally return the info they asked for.
	vector<BoardStatus>		resultVec;
	for (int32	i = 0; i <= ack.sysstatus.board().size(); i++) {
		BoardStatus				test;
		test = ack.sysstatus.board()(i);
		resultVec.push_back(test);
	}
	return (resultVec);
}

//
// setWaveformSettings
//
bool RSPport::setWaveformSettings(uint32		RCUmask,
								  uint32		mode,
								  double		frequency,
								  uint8			phase,
								  uint32		amplitude)
{
#define	SAMPLE_FREQUENCY		160.0e6

	// Construct a command
	RSPSetwgEvent		command;
	double				dblAmpl = amplitude % 100;
	command.timestamp = RTC::Timestamp(0,0);
	command.rcumask   = RCUmask;
	command.settings().resize(1);
	command.settings()(0).freq  = (uint32)
					(((frequency / SAMPLE_FREQUENCY) * ~((uint32)0) ) + 0.5);
	command.settings()(0).phase = phase;
	command.settings()(0).ampl  = (uint32) 
					(dblAmpl / 100.0 * (1<<23) + 0.5);
	command.settings()(0).nof_samples = 1024;
	if (frequency < 1e-6) {
		command.settings()(0).mode = WGSettings::MODE_OFF;
	}
	else {	// frequency = ok
		if (mode == 0) {		// forgot to set mode? assume calc mode
			command.settings()(0).mode = WGSettings::MODE_CALC;
		}
		else {
			command.settings()(0).mode = mode;
		}
	}
	command.settings()(0).preset = 0;		// PRESET_SINE

	send (&command);

	RSPSetwgackEvent	ack(receive());

	return (ack.status == SUCCESS);
}

//
// GetSubbandStats returns the signalstrength in each subband.
//
vector<double> 	RSPport::getSubbandStats(uint32	RCUmask)
{
	// Construct a query message
	RSPGetstatsEvent		question;
	question.timestamp	= RTC::Timestamp(0,0);
	question.rcumask 	= RCUmask;
	question.cache   	= true;
	question.type		= Statistics::SUBBAND_POWER;

	// send request
	send (&question);

	// wait for answer
	RSPGetstatsackEvent ack(receive());
//	bitset<MAX_N_RCUS>	mask = getRCUMask();

	if (ack.status != SUCCESS) {
		vector<double>	empty;
		return (empty);
	}
		
	// allocate any array with the right size
	int32	nrElems = ack.stats().rows() * ack.stats().columns();
	vector<double>		resultVec;
	resultVec.resize(nrElems);

	// Copy info from blitz to vector.
	blitz::Array<double,2>&		data = ack.stats();
	for (int32	i = 0; i < nrElems; i++) {
		resultVec[i] = data(0,i);
	}

	// Finally return the info they asked for.
	return (resultVec);
}

//
// getWaveformSettings
//
vector<struct WGSettings::WGRegisterType> RSPport::getWaveformSettings(uint32		RCUmask)
{
	// Construct a query message
	RSPGetwgEvent	question;
	question.timestamp	= RTC::Timestamp(0,0);
	question.rcumask 	= RCUmask;
	question.cache   	= true;

	// send question
	send (&question);

	// Wait for answer
	RSPGetwgackEvent ack(receive());
	
	// Copy info from blitz array to vector
	vector<struct WGSettings::WGRegisterType>		resultVec;
	for (int32	i = 0; i < ack.settings().size(); i++) {
		resultVec.push_back(ack.settings()(i));
		resultVec[i].ampl = resultVec[i].ampl * 100 / (1<<23);
		resultVec[i].freq = (uint32) ((double)resultVec[i].freq / (~(uint32)0) * SAMPLE_FREQUENCY);
	}
	// Finally return the info they asked for.
	return (resultVec);
}

  } // namespace RSP
} // namespace LOFAR
