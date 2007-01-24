//#  KeyValueLoggerMaster.h: 
//#
//#  Copyright (C) 2002-2003
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

#ifndef KEYVALUELOGGERMASTER_H
#define KEYVALUELOGGERMASTER_H

#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <KVL_Protocol.ph>
#include <KVLDefines.h>
#include <OTDB/OTDBconnection.h>

namespace LOFAR {
 namespace OTDB {
	class TreeValue;
 }
 namespace GCF {  
  namespace LogSys {

/**
*/

class KeyValueLoggerMaster : public TM::GCFTask
{
public:
	KeyValueLoggerMaster ();
	virtual ~KeyValueLoggerMaster ();

private: 
	// state methods
	TM::GCFEvent::TResult initial     (TM::GCFEvent& e, TM::GCFPortInterface& p);
	TM::GCFEvent::TResult operational (TM::GCFEvent& e, TM::GCFPortInterface& p);

	// data members        
	TM::GCFTCPPort  		itsListener;

	struct TClient {
		TM::GCFPortInterface*	pPort;
		unsigned long 			hourTimerID;
		uint64 					curSeqNr;
	};
	typedef map<uint8 /*clientID*/, TClient> TRegisteredClients;
	TRegisteredClients  	_clients;

	// admin members
	typedef list<TM::GCFPortInterface*> TClients;
	TClients        		_clientsGarbage;
	OTDB::OTDBconnection*	itsOTDBconn;
	OTDB::TreeValue*		itsTreeValue;
};

  } // namespace LogSys
 } // namespace GCF
} // namespace LOFAR

#endif
