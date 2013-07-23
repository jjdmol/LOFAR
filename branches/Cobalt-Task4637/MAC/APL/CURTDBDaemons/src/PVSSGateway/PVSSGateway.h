//#  PVSSGateway.h: 
//#
//#  Copyright (C) 2013
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
//#  $Id: PVSSGateway.h 19796 2012-01-17 10:06:03Z overeem $

#ifndef KEYVALUELOGGER_H
#define KEYVALUELOGGER_H

// \file PVSSGateway.h
// one_line_description

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>
#include <Common/KVpair.h>
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/RTDB_PropertySet.h>
#include <GCF/RTDB/DPservice.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  using MACIO::GCFEvent;
  namespace GCF {  
    namespace RTDBDaemons {

class PVSSGateway : public TM::GCFTask
{
public:
	explicit PVSSGateway (const string&	name);
	~PVSSGateway ();

private: 
	struct	LogClient {
		string		name;
		int32		obsID;
		int32		msgCnt;
		int32		errCnt;
		bool		valid;
		LogClient() : obsID(0),msgCnt(-10),errCnt(0),valid(false) {};
		LogClient(const string& name, int32 obsID) : name(name),obsID(obsID),msgCnt(-10),errCnt(0),valid(false) {};
	};

	// state methods
	GCFEvent::TResult initial      (GCFEvent& e, TM::GCFPortInterface& p);
	GCFEvent::TResult connect2PVSS (GCFEvent& e, TM::GCFPortInterface& p);
	GCFEvent::TResult operational  (GCFEvent& e, TM::GCFPortInterface& p);
	GCFEvent::TResult finish_state (GCFEvent& e, TM::GCFPortInterface& p);

	static void	sigintHandler(int signum);
	void	finish();

	// helper methods
	void	_registerClient (TM::GCFPortInterface&	port, const string&	name, uint32 obsID);
	void    _registerFailure(TM::GCFPortInterface&	port);
	bool	_addKVT			(const KVpair& kvp);
	PVSS::TMACValueType	_KVpairType2PVSStype(int	kvpType);

	// data members        
	TM::GCFTCPPort*			itsListener;	// application inpt
	RTDB::DPservice*		itsDPservice;	// connection to PVSS
	TM::GCFTimerPort*		itsTimerPort;	// timer
	RTDB::RTDBPropertySet*	itsPropertySet;

	typedef map<TM::GCFPortInterface*, LogClient> 	LogClientMap;
	LogClientMap 	 		itsClients;

	// admin members
	typedef list<TM::GCFPortInterface*> TClients;
	TClients        		itsClientsGarbage;

};

    } // namespace RTDBDaemons
  } // namespace GCF
} // namespace LOFAR

#endif
