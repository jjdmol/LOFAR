//#  tCableAttenuation.cc: test reading in the attenuation.conf file.
//#
//#  Copyright (C) 2009
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
#include <Common/lofar_bitset.h>
#include <Common/lofar_map.h>
#include <Common/hexdump.h>
#include "CableAttenuation.h"

using namespace LOFAR;

int main (int	argc, char*	argv[])
{
	INIT_LOGGER("tCableAttenuation");

	// good file
	CableAttenuation	CA1("tCableAttenuation.in_1");	
	LOG_DEBUG_STR("Length  50 is " << (CA1.isLegalLength( 50) ? "" : "NOT") << " a legal length");
	LOG_DEBUG_STR("Length  60 is " << (CA1.isLegalLength( 60) ? "" : "NOT") << " a legal length");
	LOG_DEBUG_STR("Length  80 is " << (CA1.isLegalLength( 80) ? "" : "NOT") << " a legal length");
	LOG_DEBUG_STR("Length 130 is " << (CA1.isLegalLength(130) ? "" : "NOT") << " a legal length");

	try {
		CableAttenuation	CA2("tCableAttenuation.in_2");
	}
	catch (Exception& ex) {
		LOG_INFO_STR("Expected exception:" << ex.what());
	}

	try {
		CableAttenuation	CA3("tCableAttenuation.in_3");
	}
	catch (Exception& ex) {
		LOG_INFO_STR("Expected exception:" << ex.what());
	}

	try {
		CableAttenuation	CA4("tCableAttenuation.in_4");
	}
	catch (Exception& ex) {
		LOG_INFO_STR("Expected exception:" << ex.what());
	}

	try {
		CableAttenuation	CA5("tCableAttenuation.in_5");
	}
	catch (Exception& ex) {
		LOG_INFO_STR("Expected exception:" << ex.what());
	}

	try {
		CableAttenuation	CA6("tCableAttenuation.in_6");
	}
	catch (Exception& ex) {
		LOG_INFO_STR("Expected exception:" << ex.what());
	}

}

