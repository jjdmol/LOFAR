//#  LoggingClient.h: PVSS-less logging processor
//#
//#  Copyright (C) 2007
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

#ifndef LOGGINGPROCESSOR_H
#define LOGGINGPROCESSOR_H

// \file 
// PVSS-less LoggingProcessor that passes the message to a PVSS-bound LoggingProcessor.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>
#include <GCF/TM/GCF_Control.h>
#include <log4cplus/helpers/socketbuffer.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace GCF {  
    namespace CUDaemons {

class LoggingClient : public TM::GCFTask
{
public:
	explicit LoggingClient (const string&	name);
	~LoggingClient ();

private: 
	// state methods
	TM::GCFEvent::TResult initial     (TM::GCFEvent& e, TM::GCFPortInterface& p);
	TM::GCFEvent::TResult operational (TM::GCFEvent& e, TM::GCFPortInterface& p);

	// helper methods
	bool _readFromPortData(TM::GCFPortInterface& 				port, 
						   log4cplus::helpers::SocketBuffer&	buf);
	string _searchClientDP(log4cplus::spi::InternalLoggingEvent&	logEvent,
						   GCFPortInterface&						port);
	void _registerFailure (GCFPortInterface&	port);
	void _loadAdmin		  (const string&	name);
	void _saveAdmin		  (const string&	name);
	bool _loadNextMessageFile();
	void _activateBuffer();
	void _processMessage(const string&	message);

	// data members        
	TM::GCFTimerPort*		itsTimerPort;
	TM::GCFTCPPort*			itsListener;
	TM::GCFTCPPort*			itsCLmaster;
	bool					itsConnected;	// to CLmaster
	uint32					itsCLMtimerID;	// timeout timer for messages.

	// client adminsitration
	struct	LogClient {
		string		DPname;
		int32		msgCnt;
		int32		errCnt;
		bool		valid;
		LogClient() : msgCnt(-10),errCnt(0),valid(false) {};
		explicit LogClient(const string& name) : DPname(name),msgCnt(-10),errCnt(0),valid(false) {};
	};
	typedef map<TM::GCFPortInterface*, LogClient> 	LogClientMap;
	LogClientMap 	 		itsClients;

	typedef list<TM::GCFPortInterface*> 			ClientsList;
	ClientsList  	      	itsClientsGarbage;

	// message administration
	struct	Message {
		string		DPname;
		string		message;
		Message() : DPname(""), message("") {};
		Message(const string&	DP, const string&	msg) : 
			DPname(DP), message(msg) {};
	};
	typedef map<int32, Message>		MsgMap;
	MsgMap				itsMsgBuffer;

	// startup parameters
	string				itsAdminFile;
	uint32				itsMaxLinesPerFile;
	uint32				itsChunkSize;

	// contents of admin file
	string				itsSurvivalFile;
	uint32				itsSurvivalLinenr;
	string				itsDrainFile;
	uint32				itsDrainLinenr;

	// sequencenumber for sending and receiving.
	int32				itsInSeqnr;
	int32				itsOutSeqnr;

};

    } // namespace CUDaemons
  } // namespace GCF
} // namespace LOFAR

#endif
