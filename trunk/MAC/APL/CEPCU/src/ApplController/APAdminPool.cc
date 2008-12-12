//#  APAdminPool.cc: Enables a poll on a collection of dataholders.
//#
//#  Copyright (C) 2002-2004
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
#include "APAdminPool.h"

namespace LOFAR {
  namespace ACC {

APAdminPool*		APAdminPool::theirAPAdminPool = 0;


//
// getInstance()
//
APAdminPool&	APAdminPool::getInstance() 
{
	LOG_TRACE_OBJ ("APAdminPool::getInstance()");

	if (theirAPAdminPool == 0) {
		theirAPAdminPool = new APAdminPool;
	}

	return (*theirAPAdminPool);
}


APAdminPool::APAdminPool() :
	itsNrAcksToRecv(0),
	itsLastCmd     (PCCmdNone),
	itsCurElement  (0)
{
}

APAdminPool::~APAdminPool()
{}

//
// void add(APAdmin*)
//
void APAdminPool::add   (APAdmin*	anAPAdmin)
{
	LOG_TRACE_RTTI_STR ("APAdminPool::add(" << anAPAdmin << ")");

	itsAPAPool.insert (itsAPAPool.begin(), anAPAdmin);
	itsReadMask.add(anAPAdmin->getSocketID());		// schedule for read
}

//
// void remove(APAdmin*)
//
void APAdminPool::remove(APAdmin*	anAPAdmin) throw(Exception)
{
	LOG_TRACE_RTTI_STR ("APAdminPool::remove(" << anAPAdmin << ")");

	iterator	iter = itsAPAPool.begin();

	while (iter != itsAPAPool.end()) {				// search dataholder
		if (*iter == anAPAdmin) {
			// Don't read, write or expect an Ack from this AP anymore
			itsReadMask.remove(anAPAdmin->getSocketID());
			markAsOffline(anAPAdmin);
			registerAck  (itsLastCmd, *iter);
			itsAPAPool.erase(iter);					// remove from pool
			setCurElement(itsCurElement);			// boundary check
			return;									// ready
		}
		++iter;
	}
	THROW(Exception, "DataHolder " << anAPAdmin << "not in pool");
}

// loop over all dataholders to see if data is ready.
// Start scan where we stopped last time.
// TODO:rewrite for select call
APAdmin*	APAdminPool::poll(time_t		waitTime)
{
	for (int i = itsCurElement; i < itsReadMask.count(); ++i) {
		LOG_TRACE_COND_STR("poll at " << i);
		if (itsAPAPool.at(i)->read()) {
			setCurElement (i+1);
			return (itsAPAPool.at(i));
		}
	}
	itsCurElement = 0;
	
	return (0);
}

//
// void writeToAll(command, options)
//
void APAdminPool::writeToAll(PCCmd			command,
							 const string&	options)
{
	// Construct a APAdmin with the info to send to all processes.
	DH_ProcControl      DHCommand;
	DHCommand.init();
	DHCommand.setCommand (command);
	DHCommand.setOptions (options);
	DHCommand.setResult  (0);
	DHCommand.pack();           // Construct the messagebuffer

	// Write the DataHolder to all Sockets in the pool.
	void*		aBuffer = DHCommand.getDataPtr();
	int32		aSize   = DHCommand.getDataSize();
	iterator	iter    = itsAPAPool.begin();
	while (iter != itsAPAPool.end()) {				// search dataholder
		if (itsOnlineMask.isSet((*iter)->getSocketID())) {
			// TODO: should we do something with the return value???
			(*iter)->write(aBuffer, aSize);
		}
		++iter;
	}

	startAckCollection(command);
}


//
// registerAck(command, apadmin)
//
bool APAdminPool::registerAck(PCCmd			aCommand,
							  APAdmin*		anAPAdmin)
{
	if (aCommand != itsLastCmd) {
		if (itsLastCmd == PCCmdNone) {
			LOG_DEBUG_STR("Process " << anAPAdmin->getName() <<
					  " is late, Ack received for " << PCCmdName(aCommand));
		}
		else {
			LOG_WARN_STR("Process " << anAPAdmin->getName() <<
					  " is out of sync, Ack received for " << PCCmdName(aCommand) << 
					  " iso " << PCCmdName(itsLastCmd));
		}
		return (false);
	}

	if (itsAckList.isSet(anAPAdmin->getSocketID())) {
		itsAckList.remove(anAPAdmin->getSocketID());
		--itsNrAcksToRecv;
	}
	return (true);
}

// cleanup APAdmins that lost connection
// When one is found the DH is removed from the pool, the search is stopped
// and the DataHolder is returned to the user to do additional cleanup.
//
// NOTE: the algorithm is not very efficient because it exits on the first
// APAdmin that is disconnected. Repeated calls to this routine will repeatedly
// check the front of the pool.
// A more efficient way to do this is with a 'cur' variable as it is solved
// in the poll method.
APAdmin* 	APAdminPool::cleanup()
{
	APAdmin*		anAPA;
	for (int i = 0; i < itsReadMask.count(); ++i) {
		anAPA = itsAPAPool.at(i);
		if (anAPA->getState() == APSfail) {
			LOG_DEBUG_STR("APAdminPool:cleanup " << i);
			remove(anAPA);					// remove it from the pool also
			return(anAPA);					// let user do other cleanup
		}
	}
	return (0);								// nothing deleted.
}

//
// operator<<
//
std::ostream&	operator<< (std::ostream& os, const APAdminPool& anAPAP)
{
	os << "Reading: " << const_cast<FdSet*>
						(&(anAPAP.itsReadMask))->getSet()->fds_bits[0]   << 
						"(" << anAPAP.itsReadMask.count()   << ")" << endl;
	os << "Writing: " << const_cast<FdSet*>
						(&(anAPAP.itsOnlineMask))->getSet()->fds_bits[0] << 
						"(" << anAPAP.itsOnlineMask.count() << ")" << endl;
	os << "AckColl: " << const_cast<FdSet*>
						(&(anAPAP.itsAckList))->getSet()->fds_bits[0]    << 
						"(" << anAPAP.itsNrAcksToRecv << ")" << endl;
	if (anAPAP.itsLastCmd != PCCmdNone) {
		os << "Command: " << anAPAP.itsLastCmd << endl;
	}

	return (os);
}

  } // namespace ACC
} // namespace LOFAR
