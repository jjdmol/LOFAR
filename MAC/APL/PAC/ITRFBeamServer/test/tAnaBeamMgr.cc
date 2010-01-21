//#  Class.cc: one_line_description
//#
//#  Copyright (C) 2010
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
#include <APL/RTCCommon/Timestamp.h>
#include <APL/IBS_Protocol/Pointing.h>

#include <blitz/array.h>
#include <ITRFBeamServer/AnalogueBeam.h>
#include <ITRFBeamServer/AnaBeamMgr.h>

using namespace LOFAR;
using namespace RTC;
using namespace BS;
using namespace IBS_Protocol;

int main(int	argc, char*	argv[]) 
{
	INIT_LOGGER("tAnaBeamMgr");

	// set up some rcuMask for the tests. Mask 1+2 or 2+3 can be scheduled at the same time.
	// 1: 0000 0000 1111
	// 2: 0011 1111 1100
	// 3: 0000 0011 1100
	// 4: 1111 0000 0000
	bitset<MAX_RCUS>		rcuMask1(0x0000000F);	// overlaps with 2 and 3
	bitset<MAX_RCUS>		rcuMask2(0x000002FC);	// overlaps all
	bitset<MAX_RCUS>		rcuMask3(0x0000003C);	// overlaps with 1 and 2
	bitset<MAX_RCUS>		rcuMask4(0x00000F00);	// overlaps with 2

	// In these first tests we test the addPointing mechanism of the Beam class.
	AnalogueBeam	beam1("beam1", rcuMask1, 1);
	LOG_DEBUG("--- POINTING TEST 1: are contiguous pointings added in the right order");
	beam1.addPointing(Pointing(0.1, 0.1,  Timestamp(1262700000,0), 1000, "J2000"));
	beam1.addPointing(Pointing(0.1, 0.11, Timestamp(1262700000+1000,0), 1100, "J2000"));
	beam1.showPointings();

	LOG_DEBUG("--- POINTING TEST 2: are gaps recognized and filled with NONE pointings");
	beam1.addPointing(Pointing(0.1, 0.2, Timestamp(1262700000+5000,0), 2200, "J2000"));
	beam1.showPointings();

	LOG_DEBUG("--- POINTING TEST 3: are overlapping pointings rejected");
	beam1.addPointing(Pointing(0.1, 0.3, Timestamp(1262700000+1200,0), 200, "J2000"));
	beam1.showPointings();
	
	LOG_DEBUG("--- POINTING TEST 4: are gaps splitted when a new pointing is inserted in the gap");
	beam1.addPointing(Pointing(0.1, 0.4, Timestamp(1262700000+3000,0), 500, "J2000"));
	beam1.showPointings();
	
	LOG_DEBUG("--- POINTING TEST 5: can we insert a pointing in 'reverse' order'");
	beam1.addPointing(Pointing(0.1, 0.5, Timestamp(1262700000-600,0), 600, "J2000"));
	beam1.showPointings();
	
	LOG_DEBUG("--- POINTING TEST 6: can we insert an everlasting pointing in the middle");
	beam1.addPointing(Pointing(0.1, 0.6, Timestamp(1262700000+4000,0), 0, "J2000"));
	beam1.showPointings();
	
	LOG_DEBUG("--- POINTING TEST 7: can we add an everlasting pointing at the end");
	beam1.addPointing(Pointing(0.1, 0.7, Timestamp(1262700000+9000,0), 0, "J2000"));
	beam1.showPointings();
	
	// In the second set of tests we test the mechanism of activating the right analogueBeams.
	// Remember that this depends on many conditions like rank, rcu overlap and consistency in
	// staying active when a beam is active.

	LOG_DEBUG("--- SCHEDULE TEST 1: is something scheduled before the starttime is reached.");
	AnaBeamMgr	beamMgr1(96, 1);
	beamMgr1.addBeam(beam1);
	beamMgr1.activateBeams(Timestamp(1262700000-1000, 0));
	beamMgr1.showAdmin();

	LOG_DEBUG("--- SCHEDULE TEST 2: is beam made active when starttime is reached");
	beamMgr1.activateBeams(Timestamp(1262700000-500, 0));	// note first pt at ...-600
	beamMgr1.showAdmin();

	LOG_DEBUG("--- SCHEDULE TEST 3: are NONE pointings also scheduled and are old pointings deleted");
	beamMgr1.activateBeams(Timestamp(1262700000+2500, 0));	
	beamMgr1.showAdmin();

	LOG_DEBUG("--- SCHEDULE TEST 4: can we add an overlapping beam with the another rank");
	AnalogueBeam	beam2("beam2", rcuMask2, 3);
	beam2.addPointing(Pointing(0.2, 0.6, Timestamp(1262700000+2200,0), 400, "AZEL"));
	beamMgr1.addBeam(beam2);
	beamMgr1.showAdmin();

	LOG_DEBUG("--- SCHEDULE TEST 5: will the second beam become active also (it should not because of overlap)");
	beamMgr1.activateBeams(Timestamp(1262700000+2500, 0));	// no change in time
	beamMgr1.showAdmin();

	LOG_DEBUG("--- SCHEDULE TEST 6: can we add an overlapping beam with a higher rank as the previous beam");
	AnalogueBeam	beam3("beam3", rcuMask3, 2);
	beam3.addPointing(Pointing(0.3, 0.6, Timestamp(1262700000+2000,0), 1400, "ITRF"));
	beamMgr1.addBeam(beam3);
	beamMgr1.showAdmin();

	LOG_DEBUG("--- SCHEDULE TEST 7: will the third beam become active also (it should not because of overlap)");
	beamMgr1.activateBeams(Timestamp(1262700000+2500, 0));	// no change in time
	beamMgr1.showAdmin();

	LOG_DEBUG("--- SCHEDULE TEST 8: add an NON overlapping beam with an equal rank as the first beam which starts earlier");
	AnalogueBeam	beam4("beam4", rcuMask4, 1);
	beam4.addPointing(Pointing(0.4, 0.6, Timestamp(1262700000+2000,0), 1400, "MOON"));
	beamMgr1.addBeam(beam4);
	beamMgr1.showAdmin();

	LOG_DEBUG("--- SCHEDULE TEST 9: will the fourth beam become active also (it should)");
	beamMgr1.activateBeams(Timestamp(1262700000+2500, 0));	// no change in time
	beamMgr1.showAdmin();

	LOG_DEBUG("--- SCHEDULE TEST 10: delete beam 1");
	beamMgr1.deleteBeam(beam1);	
	beamMgr1.showAdmin();

	LOG_DEBUG("--- SCHEDULE TEST 11: will the third beam become active also (it should)");
	beamMgr1.activateBeams(Timestamp(1262700000+2500, 0));	// no change in time
	beamMgr1.showAdmin();

	LOG_DEBUG("--- SCHEDULE TEST 12: jump into the future, all beams should disappear");
	beamMgr1.activateBeams(Timestamp(1262700000+10000, 0));
	beamMgr1.showAdmin();


}

