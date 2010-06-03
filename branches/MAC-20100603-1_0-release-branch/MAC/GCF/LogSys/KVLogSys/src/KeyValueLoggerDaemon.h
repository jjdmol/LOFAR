//#  KeyValueLoggerDaemon.h: 
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

#ifndef KEYVALUELOGGERDAEMON_H
#define KEYVALUELOGGERDAEMON_H

#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <KVL_Protocol.ph>
#include <KVLDefines.h>
//#include <PropertyLogger.h>

namespace LOFAR {
 namespace GCF {  
  namespace LogSys {

/**
*/

class KeyValueLoggerDaemon : public TM::GCFTask
{
public:
	KeyValueLoggerDaemon ();
	virtual ~KeyValueLoggerDaemon ();
    
private: 
	// state methods
    TM::GCFEvent::TResult initial     (TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult operational (TM::GCFEvent& e, TM::GCFPortInterface& p);
        
	// helper methods
    void sendEventsBuffer();
    void sendOldestCollection();
    
	// data members        
    TM::GCFTCPPort  itsListener;
    TM::GCFPort     _kvlMasterClientPort;
    typedef list<TM::GCFPortInterface*> TClients;

	// admin members
    typedef map<uint64 /*seqnr */, KVLEventCollectionEvent*> TSequenceList;
    TSequenceList   _seqList;
    TClients        _clientsGarbage;
    unsigned char   _eventsBuf[MAX_EVENTS_BUFF_SIZE];
    unsigned int    _nrOfBufferedEvents;
    unsigned int    _curEventsBufSize;
    uint8           _registerID;
    uint64          _curSeqNr;
    uint64          _oldestUnanswerdSeqNr;
//    PropertyLogger  _propertyLogger;
    bool            _waitForAnswer;
    
};
  } // namespace LogSys
 } // namespace GCF
} // namespace LOFAR

#endif
