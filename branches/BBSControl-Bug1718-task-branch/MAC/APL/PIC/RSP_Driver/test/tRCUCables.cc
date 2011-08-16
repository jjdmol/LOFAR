//#  tRCUCables.cc: test reading in the CableDelays.conf file.
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
#include "RCUCables.h"

using namespace LOFAR;

int main (int	argc, char*	argv[])
{
	INIT_LOGGER("tRCUCables");

	// good file
	RCUCables	RC1("tRCUCables.in_CableAtts", "tRCUCables.in_1");
	// Note: test it in 4 loops iso 1 because the results are easier to interpret than.
	for (int rcuMode = 0; rcuMode <= 7; rcuMode++) {
		LOG_DEBUG_STR("Largest delay for mode   " << rcuMode << ": " << RC1.getLargestDelay(rcuMode));
	}
	for (int rcuMode = 0; rcuMode <= 7; rcuMode++) {
		LOG_DEBUG_STR("Largest atten for mode   " << rcuMode << ": " << RC1.getLargestAtt  (rcuMode));
	}
	for (int rcuMode = 0; rcuMode <= 7; rcuMode++) {
		LOG_DEBUG_STR("Delay for RCU 5 in mode " << rcuMode << ": " << RC1.getDelay(5, rcuMode));
	}
	for (int rcuMode = 0; rcuMode <= 7; rcuMode++) {
		LOG_DEBUG_STR("Atten for RCU 5 in mode " << rcuMode << ": " << RC1.getAtt  (5, rcuMode));
	}

	try {
		RCUCables	RC2("tRCUCables.in_CableAtts", "tRCUCables.in_2");
	}
	catch (Exception& ex) {
		LOG_INFO_STR("Expected exception:" << ex.what());
	}

	try {
		RCUCables	RC3("tRCUCables.in_CableAtts", "tRCUCables.in_3");
	}
	catch (Exception& ex) {
		LOG_INFO_STR("Expected exception:" << ex.what());
	}

	try {
		RCUCables	RC4("tRCUCables.in_CableAtts", "tRCUCables.in_4");
	}
	catch (Exception& ex) {
		LOG_INFO_STR("Expected exception:" << ex.what());
	}

	try {
		RCUCables	RC5("tRCUCables.in_CableAtts", "tRCUCables.in_5");

	}
	catch (Exception& ex) {
		LOG_INFO_STR("Expected exception:" << ex.what());
	}

	try {
		RCUCables	RC6("tRCUCables.in_CableAtts", "tRCUCables.in_6");
	}
	catch (Exception& ex) {
		LOG_INFO_STR("Expected exception:" << ex.what());
	}
}

