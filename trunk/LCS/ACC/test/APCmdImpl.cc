//#  APCmdImpl.cc: one line description
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
#include<Common/LofarLogger.h>
#include<APCmdImpl.h>

namespace LOFAR {
  namespace ACC {

APCmdImpl::APCmdImpl()
{}

APCmdImpl::~APCmdImpl()
{}

bool	APCmdImpl::define 	 () const
{
	LOG_DEBUG("define");
	return (true);
}

bool	APCmdImpl::init 	 () const
{
	LOG_DEBUG("init");
	return (true);
}

bool	APCmdImpl::run 	 () const
{
	LOG_DEBUG("run");
	return (true);
}

bool	APCmdImpl::pause  	 (const	string&		condition) 	  const
{
	LOG_DEBUG_STR("pause: " << condition);
	return (true);
}

bool	APCmdImpl::quit  	 () const
{
	LOG_DEBUG("quit");
	return (true);
}

bool	APCmdImpl::snapshot (const string&		destination)  const
{
	LOG_DEBUG_STR("snapshot: " << destination);
	return (true);
}

bool	APCmdImpl::recover  (const string&		source) 	  const
{
	LOG_DEBUG_STR("recover: " << source);
	return (true);
}


bool	APCmdImpl::reinit	 (const string&		configID)	  const
{
	LOG_DEBUG_STR("reinit: " << configID);
	return (true);
}

// Define a generic way to exchange info between client and server.
string	APCmdImpl::askInfo   (const string& 	keylist) const
{
	LOG_DEBUG_STR("askinfo: " << keylist);
	return ("APCmdImpl: askInfo not yet implemented");
}

  } // namespace ACC
} // namespace LOFAR
