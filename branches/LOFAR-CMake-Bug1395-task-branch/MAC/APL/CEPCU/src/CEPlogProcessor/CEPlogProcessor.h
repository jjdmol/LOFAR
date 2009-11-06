//#  CEPlogProcessor.h: Daemon for catching CEP cout log streams
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_APL_CEPLOGPROCESSOR_H
#define LOFAR_APL_CEPLOGPROCESSOR_H

// \file
// Daemon for launching Application Controllers

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/Exception.h>
#include <Common/Net/Socket.h>
#include <Common/Net/FdSet.h>
#include <Common/ParameterSet.h>


namespace LOFAR {
  namespace APL {
// \addtogroup CEPCU
// @{


// The CEPlogProcessor class implements a small daemon that ...
class CEPlogProcessor
{
public:
	// Creates an CEPlogProcessor object that start listening on the port mentioned
	// in the ParameterSet.
	explicit CEPlogProcessor(const string&	progName);

	// Destructor.
	~CEPlogProcessor();

	// Its normal (never ending) loop.
	void doWork() throw (Exception);

private:
	// Copying is not allowed
	CEPlogProcessor(const CEPlogProcessor&	that);
	CEPlogProcessor& operator=(const CEPlogProcessor& that);

	void handleConnectionRequest();
	void handleDataStream(int	sid);

	//# --- Datamembers --- 
	// The listener socket to receive the requests on.
	Socket*			itsListener;

	// The parameterSet that was received during start up.
	ParameterSet*	itsParamSet;

	// File descriptor set of connected sockets
	FdSet			itsConnSet;

	// internal structure for admin for 1 stream
	typedef struct {
		Socket*		socket;
		char*		buffer;
		int			inPtr;
		int			outPtr;
	} streamBuffer_t;

	// Map containing all the streambuffers.
	map<int, streamBuffer_t>	itsLogStreams;

	// buffersize of the streambuffers.
	int		itsBufferSize;
};

// @} addgroup
  } // namespace APL
} // namespace LOFAR

#endif
