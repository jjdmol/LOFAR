//#  CEPlogProcessor.cc: Captures cout datastreams of CEP programs
//#
//#  Copyright (C) 2009
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
#include <Common/LofarLocators.h>
#include <Common/SystemUtil.h>
#include <ALC/ACRequest.h>
#include "CEPlogProcessor.h"

namespace LOFAR {
  namespace APL {

CEPlogProcessor::CEPlogProcessor(const string&	progName) :
	itsListener   (0),
	itsParamSet   (new ParameterSet)
{
	// Read in the parameterfile with network parameters.
	ConfigLocator	aCL;
	string			configFile(progName + ".conf");
	LOG_DEBUG_STR("Using parameterfile: "<< configFile <<"-->"<< aCL.locate(configFile));
	itsParamSet->adoptFile(aCL.locate(configFile));		// May throw

	// Open listener for AC requests.
	itsListener = new Socket("CEPlogProcessor",
							 itsParamSet->getString("CEPlogProcessor.portNr"),
							 Socket::TCP);

	itsBufferSize = itsParamSet->getInt("CEPlogProcessor.bufferSize", 1024);
}

CEPlogProcessor::~CEPlogProcessor()
{
	if (itsListener)   { delete itsListener; }
	if (itsParamSet)   { delete itsParamSet; }
}

void CEPlogProcessor::doWork() throw (Exception)
{
	// Prepare a fd_set for select
	itsConnSet.add(itsListener->getSid());

	LOG_INFO ("CEPlogProcessor: entering main loop");

	while (true) {
		// wait for connection request or data
		FdSet	readSet(itsConnSet);
		struct timeval	tv;					// prepare select-timer
		tv.tv_sec       = 60;				// this must be IN the while loop
		tv.tv_usec      = 0;				// because select will change tv
		int32 selResult = select(readSet.highest()+1, readSet.getSet(), 0, 0, &tv);

		// -1 may be an interrupt or a program-error.
		if ((selResult == -1) && (errno != EINTR)) {
			THROW(Exception, "CEPlogProcessor: 'select' returned serious error: " << 
							  errno << ":" << strerror(errno));
		}

		if (selResult == -1) {		// EINTR: ignore
			continue;
		}

		for (int sid = itsConnSet.lowest(); sid <= itsConnSet.highest(); sid++) {
			LOG_TRACE_FLOW_STR("testing sid " << sid);
			if (!readSet.isSet(sid)) {
				continue;
			}

			// Request for new AC?
			if (sid == itsListener->getSid()) {
				handleConnectionRequest();
			}
			else {
				handleDataStream(sid);
			}
		} // for 
	} // while
}

//
// handleConnectionRequest()
//
void CEPlogProcessor::handleConnectionRequest()
{
	// Accept the new connection
	Socket*	dataSocket = itsListener->accept(-1);
	ASSERTSTR(dataSocket,
			  "Serious problems on listener socket, exiting! : " << itsListener->errstr());
	itsConnSet.add(dataSocket->getSid());

	// give stream its own buffer.
	streamBuffer_t		stream;
	stream.socket	= dataSocket;
	stream.buffer 	= (char*)malloc(itsBufferSize);
	stream.inPtr  	= 0;
	stream.outPtr 	= 0;
	itsLogStreams[dataSocket->getSid()] = stream;
}

//
// handleDataStream(sid)
//
void CEPlogProcessor::handleDataStream(int		sid) 
{
	// read in the new bytes
	streamBuffer_t	stream		= itsLogStreams[sid];
	LOG_DEBUG_STR("handleDataStream[" << sid << "]:in=" << stream.inPtr << ", out=" << stream.outPtr);
	int	newBytes = stream.socket->read(stream.buffer + stream.inPtr, itsBufferSize - stream.inPtr);
	if (newBytes < 0) {
		LOG_ERROR_STR("read on socket " << sid << " returned " << newBytes << ". Closing connection");
		free (stream.buffer);
		stream.socket->close();
		free(stream.socket);
		itsLogStreams.erase(sid);
		itsConnSet.remove(sid);
		return;
	}
	LOG_DEBUG_STR("Received " << newBytes << " bytes at sid " << sid);
	stream.inPtr += newBytes;
	
	// process as much data as possible from the buffer.
	for (int i = stream.outPtr; i <= stream.inPtr; i++) {
		if (stream.buffer[i] != '\n') {
			continue;
		}

		stream.buffer[i] = '\0';
		LOG_INFO(formatString("SID %d:>%s<", sid, &(stream.buffer[stream.outPtr])));
		stream.outPtr = i+1;
		if (stream.outPtr >= stream.inPtr) {	// All received bytes handled?
			LOG_DEBUG("Reset of read/write pointers");
			stream.inPtr = 0;
			stream.outPtr = 0;
			itsLogStreams[sid] = stream;	// copy changes back to admin
			return;
		} 
	}

	if (stream.outPtr > (int)(0.5*itsBufferSize)) {
		// When buffer becomes full shift leftovers to the left.
		LOG_DEBUG_STR("move with: " << stream.inPtr << ", " << stream.outPtr);
		memmove (stream.buffer, stream.buffer + stream.outPtr, (stream.inPtr - stream.outPtr + 1));
		stream.inPtr -= stream.outPtr;
		stream.outPtr = 0;
	}

	itsLogStreams[sid] = stream; // copy changes back to admin
}

  } // namespace APL
} // namespace LOFAR
