//#  ACCmdImpl.cc: The actual command implementation for the A. Controller
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <time.h>
#include "ACCmdImpl.h"
#include "APAdminPool.h"

namespace LOFAR {
  namespace ACC {

ACCmdImpl::ACCmdImpl() :
	ApplControl()
{
}

// Destructor;
ACCmdImpl::~ACCmdImpl() { };


// Commands to control the application
bool	ACCmdImpl::boot  (const time_t	/* scheduleTime */,
						  const string& /* configID */ ) const
{
	LOG_WARN("boot: Should have been implemented in the statemachine");
	return (true);
}

bool	ACCmdImpl::define(const time_t		scheduleTime) const
{
	LOG_DEBUG(formatString("define(%s)", timeString(scheduleTime).c_str()));
	APAdminPool::getInstance().writeToAll(PCCmdDefine, "");
	return (true);
}

bool	ACCmdImpl::init  (const time_t		scheduleTime) const
{
	LOG_DEBUG(formatString("init(%s)", timeString(scheduleTime).c_str()));
	APAdminPool::getInstance().writeToAll(PCCmdInit, "");
	return (true);
}

bool	ACCmdImpl::run 	 (const time_t		scheduleTime) const
{
	LOG_DEBUG(formatString("run(%s)", timeString(scheduleTime).c_str()));
	APAdminPool::getInstance().writeToAll(PCCmdRun, "");
	return (true);
}

bool	ACCmdImpl::pause (const time_t		scheduleTime,
						  const time_t		waitTime,
						  const	string&		condition) const
{
	LOG_DEBUG(formatString("pause(%s,%d,%s)", timeString(scheduleTime).c_str(),
												waitTime, condition.c_str()));
	APAdminPool::getInstance().writeToAll(PCCmdPause, condition);
	return (true);
}

bool	ACCmdImpl::release  (const time_t		scheduleTime) const
{
	LOG_DEBUG(formatString("release(%s)", timeString(scheduleTime).c_str()));
	APAdminPool::getInstance().writeToAll(PCCmdRelease, "");
	return (true);
}

bool	ACCmdImpl::quit  (const time_t		scheduleTime) const
{
	LOG_DEBUG(formatString("quit(%s)", timeString(scheduleTime).c_str()));
	APAdminPool::getInstance().writeToAll(PCCmdQuit, "");
	return (true);
}

bool	ACCmdImpl::shutdown (const time_t/*	scheduleTime*/) const
{
	LOG_WARN("shutdown: Should have been implemented in the statemachine");
	return (true);
}

bool	ACCmdImpl::snapshot (const time_t		scheduleTime,
						     const string&		destination) const
{
	LOG_DEBUG(formatString("snapshot(%s,%s)", timeString(scheduleTime).c_str(),
														destination.c_str()));
	APAdminPool::getInstance().writeToAll(PCCmdSnapshot, destination);
	return (true);
}

bool	ACCmdImpl::recover  (const time_t		scheduleTime,
						     const string&		source) const
{
	LOG_DEBUG(formatString("recover(%s,%s)", timeString(scheduleTime).c_str(),
														source.c_str()));
	APAdminPool::getInstance().writeToAll(PCCmdRecover, source);
	return (true);
}


bool	ACCmdImpl::reinit (const time_t		scheduleTime,
						   const string&	configID) const
{
	LOG_DEBUG(formatString("reinit(%s,%s)", timeString(scheduleTime).c_str(),
													configID.c_str()));
	APAdminPool::getInstance().writeToAll(PCCmdReinit, configID);
	return (true);
}

bool	ACCmdImpl::replace	 (const time_t		scheduleTime,
						      const string&		processList,
						      const string&		nodeList,
						      const string&		configID) const
{
	// TODO
	return (true);
}

bool	ACCmdImpl::cancelCmdQueue () const 
{
	THROW (Exception, "We should never come in ACCmdImpl::cancelCmdQueue");
	return (true);
}

// Define a generic way to exchange info between client and server.
string	ACCmdImpl::askInfo   (const string& 	keylist) const
{
	// TODO
	return ("ACCmdImpl: askInfo not yet implemented");
}



  } // namespace ACC
} // namespace LOFAR
