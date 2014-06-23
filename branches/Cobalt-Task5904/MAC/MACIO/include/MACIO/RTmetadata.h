//#  RTmetadata.h: LCS-Common-Socket based impl to store metadata in PVSS.
//#
//#  Copyright (C) 2013-2014
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

#ifndef LOFAR_MACIO_RTMETADATA_H
#define LOFAR_MACIO_RTMETADATA_H

// \file RTmetadata.h
// LCS-Common-Socket based impl to store metadata in PVSS.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <boost/scoped_ptr.hpp>
#include <MACIO/GCF_Event.h>
#include <MACIO/EventPort.h>
#include <MACIO/KVT_Protocol.ph>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <Common/KVpair.h>
#include <Common/Thread/Thread.h>
#include <Common/Thread/Mutex.h>
#include <Common/Thread/Condition.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace MACIO {

// \addtogroup MACIO
// @{


// The RTmetadata class is a LCS/Common Socket based TCP port to make it
// possible for CEP applications to store Key-Value(_Timestamp) values in PVSS
class RTmetadata
{
public:
	// After construction, you must call start() to have already or to be
	// log()ed key-value pairs written to a PVSS Gateway.
	//
	// Pairs are buffered up to some fixed maximum. If more pairs are
	// log()ed than can be submitted by the start()ed thread, those pairs
	// are silently dropped. 
	//
	// If hostname is "", data points logged through log() will not be
	// written, since then start() does not start a thread (and LOGs this).
	// This is useful for pipeline tests that don't care about this stuff.
	RTmetadata(uint32		observationID,
		   const string&	registerName, 
		   const string&	hostname);

	~RTmetadata();

	// Start a thread (if not running) to connect and
	// send logged (queued) key-value pairs to a PVSS gateway.
	//
	// Does not necessarily have to be called before log()
	// (or at all (e.g. for unrelated tests)).
	void start();

	// log()
	// Note that events are buffered up to a maximum, then silently dropped.
	void log(const KVpair& pair);

	void log(const vector<KVpair>& pairs);

	template <typename T> 
	inline void log(const string& key, const T& value)
	{
		log(KVpair(key, value));
	}

private:
	RTmetadata();
	// Copying is not allowed
	RTmetadata(const RTmetadata& that);
	RTmetadata& operator=(const RTmetadata& that);

        void rtmLoop();
	void setupConnection();
	void sendEventsLoop();

	//# --- Datamembers ---
	static const unsigned	MAX_QUEUED_EVENTS = 1024;

	uint32			itsObsID;
	string			itsRegisterName;
	string			itsHostName;
	EventPort*		itsKVTport;
	unsigned		itsNrEventsDropped;

	// For itsThread to send from. Contains vector<KVpair> kvps.
	KVTSendMsgPoolEvent     itsLogEvents;

	// For users to log() to.
	vector<KVpair>		itsQueuedEvents;

	// Protect itsQueuedEvents from concurrent adds,
	// and from add while swapping with itsLogEvents.
	Mutex			itsQueuedEventsMutex;
	Condition		itsQueuedEventsCond;


	// Always have itsThread as the last declaration to guarantee
	// it is destructed before the data it touches.
	boost::scoped_ptr<Thread> itsThread;
};


// @}
  } // namespace MACIO
} // namespace LOFAR

#endif
