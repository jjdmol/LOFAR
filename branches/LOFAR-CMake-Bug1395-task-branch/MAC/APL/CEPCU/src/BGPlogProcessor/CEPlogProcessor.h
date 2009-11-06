//#  CEPlogProcessor.cc: Moves the operator info from the logfiles to PVSS
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
#ifndef LOFAR_APL_CEPLOGPROCESSOR_H
#define LOFAR_APL_CEPLOGPROCESSOR_H

// \file
// Daemon for launching Application Controllers

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/RTDB_PropertySet.h>

namespace LOFAR {
	using MACIO::GCFEvent;
	using GCF::TM::GCFTask;
	using GCF::TM::GCFTCPPort;
	using GCF::TM::GCFTimerPort;
	using GCF::TM::GCFPortInterface;
	using GCF::RTDB::RTDBPropertySet;
	namespace APL {

// \addtogroup CEPCU
// @{


// The CEPlogProcessor class implements a small daemon that ...
class CEPlogProcessor : public GCFTask
{
public:
	explicit CEPlogProcessor(const string&	cntlrName);
	~CEPlogProcessor();

	// its processing states
	GCFEvent::TResult initial_state		(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult createPropertySets(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult startListener		(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult operational		(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult finish_state		(GCFEvent& event, GCFPortInterface& port);

	// Interrupthandler for switching to the finish state when exiting the program
	static void signalHandler (int signum);
	void		finish();
	
private:
	// Copying is not allowed
	CEPlogProcessor();
	CEPlogProcessor(const CEPlogProcessor&	that);
	CEPlogProcessor& operator=(const CEPlogProcessor& that);

	// Admin functions
	void	 _deleteStream	  (GCFPortInterface&	port);
	void	 _handleConnectionRequest();

	// Routines for processing the loglines.
	void	 _handleDataStream  (GCFPortInterface*	port);
	string	 _getProcessID	    (char*	cString);
	void	 _processLogLine    (char*	cString);
	void	 _processIONProcLine(int	processNr, char*	cstring);
	void	 _processCN_ProcLine(int	processNr, char*	cstring);
	void	 _processStorageLine(int	processNr, char*	cstring);

	//# --- Datamembers --- 
	// The listener socket to receive the requests on.
	GCFTCPPort*		itsListener;

	RTDBPropertySet*	itsOwnPropertySet;
	GCFTimerPort*		itsTimerPort;

	// internal structure for admin for 1 stream
	typedef struct {
		GCFTCPPort*	socket;
		char*		buffer;
		int			inPtr;
		int			outPtr;
	} streamBuffer_t;

	// internal structure for lse based logging
	typedef struct {
	    vector<string>              timeStr;
	    vector<int>                 count;
	    vector<string>              dropped;
	} logBuffer_t;
	  


	// Map containing all the streambuffers.
	map<GCFPortInterface*, streamBuffer_t>	itsLogStreams;

	vector<RTDBPropertySet*>	itsInputBuffers;
	vector<RTDBPropertySet*>	itsAdders;
	vector<RTDBPropertySet*>	itsStorage;
	vector<int>                 itsDroppingCount;
    vector<logBuffer_t>         itsStorageBuf;


	// values read from the conf file.
	int					itsNrInputBuffers;
	int					itsNrAdders;
	int                 itsNrStorage;
	int					itsBufferSize;
};

// @} addgroup
  } // namespace APL
} // namespace LOFAR

#endif
