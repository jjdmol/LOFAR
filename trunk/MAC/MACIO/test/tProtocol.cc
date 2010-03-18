//#  tProtocol.cc
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
#include <Common/StringUtil.h>
#include "testprotocol.ph"
#include "Echo_Protocol.ph"

using namespace LOFAR;
using namespace LOFAR::MACIO;

int main (int	/*argc*/, char* argv[]) 
{
	INIT_LOGGER(argv[0]);

	LOG_INFO_STR ("TESTPROTOCOL has id: " << TESTPROTOCOL);

	TEST_PTCTestInEvent		inEvent;

	LOG_INFO("Trying to print an errormsg before registering the protocol");
	LOG_INFO_STR("errornr: " << TEST_PTC_WINDOWS_ERR << " = " 
							 << errorName (TEST_PTC_WINDOWS_ERR));
	LOG_INFO_STR("eventname of in-event: " << eventName(inEvent));

	LOG_INFO("--- Registering the testprotocol ---");
	registerProtocol (TESTPROTOCOL, TESTPROTOCOL_STRINGS);
	LOG_INFO(formatString("errornr: %d = %s", TEST_PTC_WINDOWS_ERR, 
									errorName (TEST_PTC_WINDOWS_ERR).c_str()));
	LOG_INFO_STR("signalname of in-event: " << eventName(inEvent));

	LOG_INFO ("Registering a second protocol");
	registerProtocol (ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);
	
	EchoPingEvent		pingEvent;
	LOG_INFO_STR("signalname of ping-event: " << eventName(pingEvent));

	return (0);
}

