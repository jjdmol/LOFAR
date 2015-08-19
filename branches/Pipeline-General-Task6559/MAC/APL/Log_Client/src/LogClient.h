//#  LogClient.h: PVSS-less logging processor
//#
//#  Copyright (C) 2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
// PVSS-less LogProcessor that passes the message to a PVSS-bound LogProcessor.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>
#include <GCF/TM/GCF_Control.h>
#include <log4cplus/helpers/socketbuffer.h>
#include <log4cplus/spi/loggingevent.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
	using namespace GCF::TM;
    namespace Log_Client {

class LogClient : public GCFTask
{
public:
	explicit LogClient (const string&	name);
	~LogClient ();

private: 
	// state methods
	GCFEvent::TResult initial     (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult operational (GCFEvent& e, GCFPortInterface& p);

	// helper methods
	bool _readFromPortData(GCFPortInterface& 				port, 
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
	GCFTimerPort*		itsTimerPort;
	GCFTCPPort*			itsListener;
	GCFTCPPort*			itsCLmaster;
	bool				itsConnected;	// to CLmaster
	uint32				itsCLMtimerID;	// timeout timer for messages.

	// client adminsitration
	struct	LogProc {
		// Max. number of message that will be searched for identification string
  		static const int32	maxInitMsgCount = 20;
		string		DPname;
		int32		msgCnt;
		int32		errCnt;
		bool		valid;
		LogProc() : msgCnt(-maxInitMsgCount),errCnt(0),valid(false) {};
		explicit LogProc(const string& name) : DPname(name),msgCnt(-maxInitMsgCount),errCnt(0),valid(false) {};
	};
	typedef map<GCFPortInterface*, LogProc> 	LogProcMap;
	LogProcMap 	 		itsClients;

	typedef list<GCFPortInterface*> 			ClientsList;
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
	string				itsMasterHost;
	bool				itsInTestMode;

	// contents of admin file
	string				itsSurvivalFile;
	uint32				itsSurvivalLinenr;
	string				itsDrainFile;
	uint32				itsDrainLinenr;

	// sequencenumber for sending and receiving.
	int32				itsInSeqnr;
	int32				itsOutSeqnr;

};

    } // namespace Log_Client
} // namespace LOFAR

#endif
