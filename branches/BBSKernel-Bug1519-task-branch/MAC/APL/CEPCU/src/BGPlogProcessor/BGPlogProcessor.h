//#  BGPlogProcessor.h: Daemon for catching CEP cout log streams
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

#ifndef LOFAR_APL_BGPLOGPROCESSOR_H
#define LOFAR_APL_BGPLOGPROCESSOR_H

// \file
// Daemon for launching Application Controllers

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
  using GCF::TM::GCFTask;
  using GCF::TM::GCFTCPPort;
  using GCF::TM::GCFPortInterface;
  namespace APL {
// \addtogroup CEPCU
// @{


// The BGPlogProcessor class implements a small daemon that ...
class BGPlogProcessor : public GCFTask
{
public:
	// Creates an BGPlogProcessor object that start listening on the port mentioned
	// in the ParameterSet.
	explicit BGPlogProcessor(const string&	progName);

	// Destructor.
	~BGPlogProcessor();

	// its processing states
	GCFEvent::TResult startListener(GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult operational  (GCFEvent& e, GCFPortInterface& p);
	
private:
	// Copying is not allowed
	BGPlogProcessor(const BGPlogProcessor&	that);
	BGPlogProcessor& operator=(const BGPlogProcessor& that);

	void	_handleConnectionRequest();
	void	_handleDataStream(GCFPortInterface*	port);
	string	_getProcessID	 (char*	cString);
	void 	_processLogLine  (char*	cString);

	//# --- Datamembers --- 
	// The listener socket to receive the requests on.
	GCFTCPPort*		itsListener;

	// internal structure for admin for 1 stream
	typedef struct {
		GCFTCPPort*	socket;
		char*		buffer;
		int			inPtr;
		int			outPtr;
	} streamBuffer_t;

	// Map containing all the streambuffers.
	map<GCFPortInterface*, streamBuffer_t>	itsLogStreams;

	// buffersize of the streambuffers.
	int		itsBufferSize;
};

// @} addgroup
  } // namespace APL
} // namespace LOFAR

#endif
