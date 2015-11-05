//#  APAdminPool.h: Administation of the AP's for the Appl. controller
//#
//#  Copyright (C) 2004
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

#ifndef LOFAR_ACCBIN_APADMINPOOL_H
#define LOFAR_ACCBIN_APADMINPOOL_H

// \file
// Collection of APAdmin classes used by the Application Controller for
// managing the Application Processes.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_vector.h>
#include <Common/Net/FdSet.h>
#include "APAdmin.h"

namespace LOFAR {
  namespace ACC {
// \addtogroup ACCbin
// @{

// The APAdminPool object manages a pool of APAdmin objects. An APAdmin object
// is a pair of a DataHolder and a Socket. The surplus value of the APAdminPool
// is that is can perform an action of all the APAdmin objects it holds.
// Since the APAdminPool has knowledge of all the Processes and its connection
// state it is extended with administrative functions for collecting ACK
// messages.
class APAdminPool
{
	typedef vector<APAdmin*>			APAList;
	typedef vector<APAdmin*>::iterator	iterator;

public:
	// APAdminPool is a singleton class.
	static APAdminPool&		getInstance();
	virtual 				~APAdminPool();

	// \name Maintenance on the APAdmin members
	// Functions for maintaining the pool of APAdmin objects.
	// @{

	// Adds the given APAdmin to the pool.
	void add   (APAdmin*	anAPAdmin);

	// Removes the given APAdmin from the pool. Throws an exception if the
	// APAdmin is not in the pool.
	void remove(APAdmin*	anAPAdmin) throw(Exception);
	// @}
	
	// \Managing process state
	// The APAdminPool knows of every connected process if it is ready to
	// receive messages of not.
	// @{
	
	// The given APAdmin is ready to receive commands
	void markAsOnline (APAdmin*		anAPAdmin);

	// Don't send any more messages to this APAdmin.
	void markAsOffline(APAdmin*		anAPAdmin);
	// @}

	// \name Functions for Ack administration
	// @{

	// When calling \c startAckCollection all processes that are marked as
	// 'online' are also marked 'shouldSendAnAck'. The Acks that are send by
	// the processes should be a respons on command \c aCommand.
	void startAckCollection(PCCmd			aCommand);

	// Tell the APAdminPool that on this APAdmin an Ack message was received
	// for the given command. Tell the APAdminPool that on this APAdmin an Ack
	// message was received. Registering an Ack for the wrong command has no 
	// effect.
	bool registerAck       (PCCmd			aCommand,
							APAdmin*		anAPAdmin);

	// Cancel the collection of Acks. When Acks are registered after this call
	// they have no effect anymore.
	void stopAckCollection ();

	// Check if all Acks are received.
	bool allAcksReceived   ();
	// @}

	// \name Actions on the pool
	// The following actions are executed on all elements of the pool.
	// @{

	// Continues its previous poll-sequence on the elements in its pool.
	// Returns with a pointer to the APAdmin that has received a complete
	// message. Returns 0 if the end of the pool is reached.<br>
	// To do a single read on all elements of the pool:
	// \code
	//   while ((activeAP = myAPAdminPool.poll()) {
	//      ... do something with the data of the 'activeAP'
	//   }
	// \endcode
	APAdmin*	poll        (time_t			waitTime);

	// Continues its previous cleanup-sequence on the elements in its pool.
	// Returns with a pointer to the APAdmin that has received a complete
	// message. Returns 0 if the end of the pool is reached.<br>
	// To do a single read on all elements of the pool:
	// \code
	//   while ((deadAP = myAPAdminPool.cleanup()) {
	//      ... do some final task with 'deadAP'
	//      delete deadAP
	//   }
	// \endcode
	APAdmin*	cleanup     ();

	// Contructs a DH_ProcControl object from the passed arguments and sends
	// this dataholder to all elements in its pool.
	// After sending it to all elements \c startAckCollection is called to
	// remember from with processes a respons is expected.
	void 		writeToAll(PCCmd			command,
						   const string&	options);
	// @}

	// \name Accessor methods
	// @{

	// Returns the number of processes in its pool.
	uint16		processCount();

	// Returns the number of processes in its pool that have told that they
	// are ready to receive data.
	uint16		onlineCount ();
	// @}

	friend std::ostream& operator<< (std::ostream& os, const APAdminPool& anAPAP);

private:
	// Not default contructable.
	APAdminPool();

	// Copying is not allowed.
	APAdminPool(const APAdminPool&	that);

	// Copying is not allowed.
	APAdminPool& operator=(const APAdminPool& that);

	// Internal command for adjusting the index for the poll method.
	void setCurElement(uint16	aValue);

	// The APAdminPool is a singleton.
	static		APAdminPool*		theirAPAdminPool;

	APAList		itsAPAPool;			// vector of APAdmin objects
	FdSet		itsReadMask;		// selector mask
	FdSet		itsOnlineMask;		// Procs that are ready to receive commands
	FdSet		itsAckList;			// Procs still to receive an Ack from
	uint16		itsNrAcksToRecv;	// Nr of Acks still to receive.
	PCCmd		itsLastCmd;			// Last/current outstanding AP command
	uint16		itsCurElement;		// Last element we polled.
};

//# -------------------- inline functions --------------------
//#
//# setCurElement(value)
//#
inline void APAdminPool::setCurElement(uint16	aValue)
{
	if (aValue >= itsReadMask.count()) {		// check upper boundary
		itsCurElement = 0;
	}
	else {
		itsCurElement = aValue;
	}
}

//#
//# processCount
//#
inline uint16 APAdminPool::processCount()
{
	return (itsReadMask.count());
}

//#
//# onlineCount
//#
inline uint16 APAdminPool::onlineCount()
{
	return (itsOnlineMask.count());
}

//#
//# markAsOnline(APAdmin*)
//#
inline void APAdminPool::markAsOnline(APAdmin*		anAPAdmin)
{
	itsOnlineMask.add(anAPAdmin->getSocketID());		// schedule for writes
}
	
//#
//# markAsOffLine(apadmin)
//#
inline void APAdminPool::markAsOffline(APAdmin*		anAPAdmin)
{
	itsOnlineMask.remove(anAPAdmin->getSocketID());
}
	
//#
//# startAckColection(command)
//#
inline void APAdminPool::startAckCollection(PCCmd  aCommand) 
{
	itsAckList      = itsOnlineMask;
	itsNrAcksToRecv = itsOnlineMask.count();
	itsLastCmd      = aCommand;
}

//#
//# stopAckCollection()
//#
inline void APAdminPool::stopAckCollection()
{
	itsAckList.clear();
	itsNrAcksToRecv = 0;
	itsLastCmd      = PCCmdNone;
}

//#
//# allAcksReceived
//#
inline bool APAdminPool::allAcksReceived()
{
	return (itsLastCmd != PCCmdNone && itsNrAcksToRecv == 0);
}

// @} addgroup
  } // namespace ACC
} // namespace LOFAR
#endif
