//#  ACCProcessList.cc: ACCProcessList is used internally by the AC
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

#include <stdio.h>
#include <Common/LofarLogger.h>
#include <ACC/ACCProcessList.h>

namespace LOFAR
{

//#-------------------------- creation and destroy ---------------------------
//#
//# Default constructor
//#
ACCProcessList::ACCProcessList()  {} 

//#
//# Copying is allowed.
//#
ACCProcessList::ACCProcessList(const ACCProcessList& that) :
	list<ACCProcess> (that)
{ }

//#
//# operator= copying
//#
ACCProcessList& ACCProcessList::operator=(const ACCProcessList& that)
{
	if (this != &that) {
		list<ACCProcess>::operator= (that);
	}
	return (*this);
}

//#
//#	Destructor
//#
ACCProcessList::~ACCProcessList()
{}

//#
//# registerProc(theProc)
//#
void ACCProcessList::registerProcess(const ACCProcess&	theProc) {
	insert(end(), theProc);
}

//#
//# sendAll	(theEventType)
//#
//# Constructs an event of the given type and send it to all processes
//#
//# Note: the event time is always set to 0 which is allowed because the
//#		  the demo PC does nothing with the time.
void ACCProcessList::sendAll	(const	ACCEvent		theEventType)
{
	char	paramBuffer[1024];
	sprintf (paramBuffer, "event.type=%d\nevent.time=0\nevent.info=none\n",
															theEventType);

	iterator		PI = begin();
	while (PI != end()) {
		if (PI->state() == ACCProcess::Running) {
			PI->socket().send(paramBuffer, strlen(paramBuffer));
		}
		*PI++;								//# hop to next process
	}
}

//#
//# shutdownAll
//#
//# closes all sockets
void ACCProcessList::shutdownAll()
{
	iterator		PI = begin();
	while (PI != end()) {
		if (PI->state() == ACCProcess::Running) {
			PI->socket().disconnect();
		}
		*PI++;								//# hop to next process
	}
}

} // namespace LOFAR
