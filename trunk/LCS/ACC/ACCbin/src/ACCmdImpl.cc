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
#include <time.h>
#include <Common/StringUtil.h>
#include <ACC/ACCmdImpl.h>
#include <ACC/APAdminPool.h>

namespace LOFAR {
  namespace ACC {

ACCmdImpl::ACCmdImpl() :
	ApplControl()
{
}

// Destructor;
ACCmdImpl::~ACCmdImpl() { };

// Commands to control the application
bool	ACCmdImpl::boot  (const time_t		scheduleTime,
						  const string&		configID) const
{

	LOG_DEBUG(formatString("boot(%s,%s)", timeString(scheduleTime).c_str(), 
														configID.c_str()));
	// TODO
	// Communicate with the Nodemanager to powerup a couple of nodes.
	// The nodes are specified in the paramfile or in the nodelist: TODO
	return(true);
}

bool	ACCmdImpl::define(const time_t		scheduleTime) const
{
	LOG_DEBUG(formatString("define(%s)", timeString(scheduleTime).c_str()));
	APAdminPool::getInstance().writeToAll(PCCmdDefine, 0x12345678, "some options?");

	// TODO
	return(true);
}

bool	ACCmdImpl::init  (const time_t		scheduleTime) const
{
	// TODO
	return(true);
}

bool	ACCmdImpl::run 	 (const time_t		scheduleTime) const
{
	// TODO
	return(true);
}

bool	ACCmdImpl::pause (const time_t		scheduleTime,
						  const time_t		waitTime,
						  const	string&		condition) const
{
	// TODO
	return(true);
}

bool	ACCmdImpl::quit  (const time_t		scheduleTime) const
{
	// TODO
	return(true);
}

bool	ACCmdImpl::shutdown (const time_t		scheduleTime) const
{
	// TODO
	return(true);
}

bool	ACCmdImpl::snapshot (const time_t		scheduleTime,
						     const string&		destination) const
{
	// TODO
	return(true);
}

bool	ACCmdImpl::recover  (const time_t		scheduleTime,
						     const string&		source) const
{
	// TODO
	return(true);
}


bool	ACCmdImpl::reinit (const time_t		scheduleTime,
						   const string&		configID) const
{
	// TODO
	return(true);
}

bool	ACCmdImpl::replace	 (const time_t		scheduleTime,
						  const string&		processList,
						  const string&		nodeList,
						  const string&		configID) const
{
	// TODO
	return(true);
}


// Define a generic way to exchange info between client and server.
string	ACCmdImpl::askInfo   (const string& 	keylist) const
{
	// TODO
	return ("ACCmdImpl: askInfo not yet implemented");
}



  } // namespace ACC
} // namespace LOFAR
