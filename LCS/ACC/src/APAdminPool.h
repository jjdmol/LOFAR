//#  APAdminPool.h: one line description
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
//#  $Id$

#ifndef ACC_APADMINPOOL_H
#define ACC_APADMINPOOL_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_vector.h>
#include <ACC/APAdmin.h>

namespace LOFAR {
  namespace ACC {

//# Forward Declarations
//class forward;


// Description of class.
class APAdminPool
{
	typedef vector<APAdmin*>			APAList;
	typedef vector<APAdmin*>::iterator	iterator;

public:
	// APAdminPool is a singleton class.
	static APAdminPool&		getInstance();
	virtual 				~APAdminPool();

	// add/remove an APAdmin to/from the pool
	void add   (APAdmin*	anAPAdmin);
	void remove(APAdmin*	anAPAdmin) throw(Exception);
	
	// given APAdmin is ready to receive commands
	void APAdminPool::markAsOnline (APAdmin*		anAPAdmin);
	void APAdminPool::markAsOffline(APAdmin*		anAPAdmin);

	// functions for Ack administration
	void startAckCollection(PCCmd			aCommand);
	void registerAck       (PCCmd			aCommand,
							APAdmin*		anAPAdmin);
	void stopAckCollection ();
	bool allAcksReceived   ();

	APAdmin*	poll      (time_t			waitTime);
	APAdmin*	cleanup   ();
	uint16		size      ();
	void 		writeToAll(PCCmd			command,
						   time_t			waitTime,
						   const string&	options);

	friend std::ostream& operator<< (std::ostream& os, const APAdminPool& anAPAP);

private:
	APAdminPool();
	APAdminPool(const APAdminPool&	that);
	APAdminPool& operator=(const APAdminPool& that);

	void setCurElement(uint16	aValue);

	// for singleton
	static		APAdminPool*		theirAPAdminPool;

	APAList		itsAPAPool;			// vector of APAdmin objects
	uint16		itsNrElements;		// nr elements in vector
	fd_set		itsReadMask;		// selector mask
	uint		itsNrOnline;		// Nr of processes online
	fd_set		itsOnlineMask;		// Procs that are ready to receive commands
	uint16		itsNrAcksToRecv;	// Nr of Acks still to receive.
	fd_set		itsAckList;			// Procs still to receive an Ack from
	PCCmd		itsLastCmd;			// Last/current outstanding AP command

	uint16		itsCurElement;		// TODO to be deleted
};

// -------------------- inline functions --------------------
inline void APAdminPool::setCurElement(uint16	aValue)
{
	if (aValue >= itsNrElements) {				// check upper boundary
		itsCurElement = 0;
	}
	else {
		itsCurElement = aValue;
	}
}

inline uint16 APAdminPool::size()
{
	return (itsNrElements);
}

//
// void markAsOnline(APAdmin*)
//
inline void APAdminPool::markAsOnline(APAdmin*		anAPAdmin)
{
	FD_SET(anAPAdmin->getSocketID(), &itsOnlineMask);		// schedule for writes
	++itsNrOnline;
}
	
//
// void markAsOfline(APAdmin*)
//
inline void APAdminPool::markAsOffline(APAdmin*		anAPAdmin)
{
	FD_CLR(anAPAdmin->getSocketID(), &itsOnlineMask);		// schedule for writes
	--itsNrOnline;
}
	
inline void APAdminPool::startAckCollection(PCCmd  aCommand) 
{
	itsAckList      = itsOnlineMask;
	itsNrAcksToRecv = itsNrOnline;
	itsLastCmd      = aCommand;
}

inline void APAdminPool::registerAck(PCCmd			aCommand,
									 APAdmin*		anAPAdmin)
{
	ASSERTSTR (aCommand == itsLastCmd, "Process" << anAPAdmin->getName() <<
		"is out of sync, Ack received for " << aCommand << "iso " << itsLastCmd);

	if (FD_ISSET(anAPAdmin->getSocketID(), &itsAckList)) {
		FD_CLR(anAPAdmin->getSocketID(), &itsAckList);
		--itsNrAcksToRecv;
	}
}

inline void APAdminPool::stopAckCollection()
{
	FD_ZERO(&itsAckList);
	itsNrAcksToRecv = 0;
	itsLastCmd      = PCCmdNone;
}

inline bool APAdminPool::allAcksReceived()
{
	return (itsLastCmd != PCCmdNone && itsNrAcksToRecv == 0);
}


  } // namespace ACC
} // namespace LOFAR
#endif
