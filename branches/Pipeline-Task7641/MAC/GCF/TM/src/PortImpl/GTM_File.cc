//#  GTM_File.cc: base class for all sockets
//#
//#  Copyright (C) 2002-2003
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

#include "GTM_File.h"
#include "GTM_FileHandler.h"
#include <GCF/TM/GCF_RawPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Scheduler.h>
//#include <GTM_Defines.h>
#include <GCF/TM/GCF_Protocols.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {

GTMFile::GTMFile(GCFRawPort& port) :
	_fd(-1),
	_pHandler(0),
	_port(port)
{
	_pHandler = GTMFileHandler::instance();
	ASSERT(_pHandler);
	itsScheduler = GCFScheduler::instance();
	ASSERT(itsScheduler);
}

GTMFile::~GTMFile()
{
	close();
	GTMFileHandler::release();
	_pHandler = 0;
	itsScheduler = 0;
}

bool GTMFile::close()
{
	bool result(true);

	if (_fd > -1) { 
		ASSERT(_pHandler);
		_pHandler->deregisterFile(*this);
		result = (::close(_fd) == 0);
		if (!result) {
			LOG_ERROR(formatString ( "::close, error: %s", strerror(errno)));
			// there is nothing we can do at this point, since we cannot know
			// whether the fd is still valid.
		}

		_fd = -1;
	}
	return result;
}

int GTMFile::setFD(int fd)
{
	if (fd >= 0) {
		if (_fd > -1 && _fd != fd) {
			close();
		}
		_fd = fd;
		ASSERT(_pHandler);
		_pHandler->registerFile(*this);
	}
	return (fd);    
}

void GTMFile::doWork()
{
	unsigned long bytesRead = 0;

	if (ioctl(_fd, FIONREAD, &bytesRead) > -1) {
		if (bytesRead == 0) {
			GCFEvent discoEvent(F_DISCONNECTED);
			itsScheduler->queueEvent(0, discoEvent, &_port);
//			_port.dispatch(e);    
		}
		else {
			GCFEvent dataEvent(F_DATAIN);
			itsScheduler->queueEvent(0, dataEvent, &_port);
//			_port.dispatch(e);
		}
	}
	else {
		ASSERT(_port.getTask());
			LOG_FATAL(LOFAR::formatString ("%s(%s): Error in 'ioctl' on socket fd %d: %s",
						_port.getTask()->getName().c_str(), 
						_port.getName().c_str(), 
						_fd, 
						strerror(errno)));        
	}
}


void GTMFile::setBlocking(bool	blocking) const
{
	if (_fd <= -1)
		return;

	int flags = fcntl(_fd, F_GETFL);

	if (blocking) {
		fcntl(_fd, F_SETFL, flags & ~O_NONBLOCK);
	} else {
		fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
	}

	LOG_DEBUG_STR("FD " << _fd << " set to " << (blocking ? "blocking" : "non blocking"));
}

//
// forwardEvent(event)
//
void GTMFile::forwardEvent (LOFAR::MACIO::GCFEvent& event) 
{
	itsScheduler->queueEvent(0, event, &_port);
}

    } // namespace TM
  } // namespace GCF
} // namespace LOFAR
