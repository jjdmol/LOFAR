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
  namespace RSP {

using	GCF::TM::GCFEvent;

//
// RSPport (host, port)
//
RSPport::RSPport(string	aHostname) :
	itsPort(24001),		// FOR THE TIME BEING!!!
	itsHost(aHostname),
	itsSocket(new Socket("RSPsocket", itsHost, toString(itsPort)))
{
	itsSocket->connect(-1);			// try to connect, wait max 1 second
	itsSocket->setBlocking(true);	// no other tasks, do rest blocking
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
// getStatus() : struct BoardStatus
//
vector<BoardStatus> RSPport::getBoardStatus(uint32	RCUmask)
{
	// Construct a query message
	RSPGetstatusEvent	question;
//	question.rcumask = xxx;
	question.cache   = 0;

	// Serialize the message and write buffer to port
	uint32	packSize;
	void* buf = question.pack(packSize);
	int32 btsWritten = itsSocket->write(buf, packSize);
	ASSERTSTR(btsWritten == (int32)packSize, 
			  "Only " << btsWritten << " of " << packSize << " bytes written");

	// First read header if answer:
	// That is signal field + length field.
	GCFEvent	header;
	int32		btsRead;
	btsRead = itsSocket->read((void*) &header.signal, sizeof(header.signal));
	ASSERTSTR(btsRead == sizeof(header.signal), "Only " << btsRead << " of " 
						<< sizeof(header.signal) << " bytes of header read");
	btsRead = itsSocket->read((void*) &header.length, sizeof(header.length));
	ASSERTSTR(btsRead == sizeof(header.length), "Only " << btsRead << " of " 
						<< sizeof(header.length) << " bytes of header read");

	LOG_DEBUG("Header received");

	// Is there a payload in the message? This should be the case!
	int32	remainingBytes = header.length;
	LOG_DEBUG_STR(remainingBytes << " bytes to get next");
	if (remainingBytes <= 0) {
		vector<BoardStatus>		empty;
		return(empty);
	}

	// create a buffer to receive the whole message
	GCFEvent*	fullAnswer = 0;
	char*	answerBuf = new char[sizeof(header) + remainingBytes];
	fullAnswer = (GCFEvent*) answerBuf;

	// copy received header
	memcpy(answerBuf, &header, sizeof(header));	
	
	// read remainder
	btsRead = itsSocket->read(answerBuf + sizeof(header), remainingBytes);
	ASSERTSTR(btsRead == remainingBytes,
		  "Only " << btsRead << " bytes of msg read: " << remainingBytes);

	hexdump(answerBuf + sizeof(header), remainingBytes);

	// Convert is to the right answer class
	RSPGetstatusackEvent ack(*fullAnswer);
	
	// Finally return the info they asked for.
	vector<BoardStatus>		resultVec;
	for (uint32	i = 0; i <= ack.sysstatus.board().size(); i++) {
		BoardStatus				test;
		test = ack.sysstatus.board()(i);
		resultVec.push_back(test);
	}
	return (resultVec);
}

//
// setWaveformSettings
//
bool setWaveformSettings(uint32		RCUmask,
						 uint32		mode,
						 uint32		frequency,
						 uint32		amplitude)
{
	// NOT YET IMPLEMENTED
	return (false);
}

//
// GetSubbandStats returns the signalstrength in each subband.
//
vector<double> 	getSubbandStats(uint32	RCUmask)
{
	// NOT YET IMPLEMENTED

	vector<double>	empty;

	return (empty);
}


  } // namespace RSP
} // namespace LOFAR
