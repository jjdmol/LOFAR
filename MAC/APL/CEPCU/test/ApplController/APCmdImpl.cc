//#  APCmdImpl.cc: example implemenation of a derived ProcessControl class.
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
#include <PLC/PCCmd.h>
#include <APCmdImpl.h>
#include <Common/lofar_tribool.h>

namespace LOFAR {
  namespace ACC {

APCmdImpl::APCmdImpl(const string&	aProcessID) :
	ProcessControl(aProcessID),
	itsRunCounter(0)
{}

APCmdImpl::~APCmdImpl()
{}

// define()
tribool	APCmdImpl::define()
{
	LOG_DEBUG("define/claim");
	return (true);
}

// init()
tribool	APCmdImpl::init()
{
	LOG_DEBUG("init/prepare");
	return (true);
}

// run()
tribool	APCmdImpl::run()
{
	LOG_DEBUG("run");

	// just to show how 'clearRunState' works we use a runcounter
	// that is set during the pause command. When this counter is set
	// we lower it every run until it reached 0.
	// After we called clearRunState no more run may occure.
	if (itsRunCounter) {
		if (--itsRunCounter == 0) {
			LOG_DEBUG("clearing runstate");
			clearRunState();
		}
	}

	// to test the metadata path we send some metadata when the counter reached 1
	if (itsRunCounter == 1) {
		LOG_DEBUG("Sending metadata about runCounter");
		ParameterSet    resultSet;
		string          resultBuffer;
		resultSet.add(KVpair(itsProcID+".runCounter",
							 string("1"),
							 true));
		sendResultParameters(resultSet);
	}

	return (true);
}

// pause(condition)
tribool	APCmdImpl::pause(const	string&		condition)
{
	LOG_DEBUG_STR("pause: " << condition);
	if (condition == PAUSE_OPTION_NOW) {
		// nothing to do
	}
	else if (condition == PAUSE_OPTION_ASAP) {
		// just for testing accept 3 more runs.
		itsRunCounter = 3;
	}
	else {
		// condition is a timestamp
		// just for testing accept 5 more runs.
		itsRunCounter = 5;
	}
	return (true);
}

// release()
tribool	APCmdImpl::release()
{
	LOG_DEBUG("release");
	return (indeterminate);	// so we test this too
}

// quit()
tribool	APCmdImpl::quit()
{
	LOG_DEBUG("quit");
	return (true);
}

// the following calls are not used yet by ACC.

tribool	APCmdImpl::snapshot(const string&		destination)
{
	LOG_DEBUG_STR("snapshot: " << destination);
	return (true);
}

tribool	APCmdImpl::recover(const string&		source)
{
	LOG_DEBUG_STR("recover: " << source);
	return (true);
}


tribool	APCmdImpl::reinit(const string&		configID)
{
	LOG_DEBUG_STR("reinit: " << configID);
	return (true);
}

// Define a generic way to exchange info between client and server.
string	APCmdImpl::askInfo(const string& 	keylist)
{
	LOG_DEBUG_STR("askinfo: " << keylist);
	return ("APCmdImpl: askInfo not yet implemented");
}

  } // namespace ACC
} // namespace LOFAR
