//
//  tNenuFarIO.cc: Implements an echo server
//
//  Copyright (C) 2014
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id: tNenuFarIO.cc 23533 2013-01-22 15:38:33Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/hexdump.h>
#include "../src/NenuFarAdmin.h"
#include "../src/NenuFarIO.h"
#include "tNenuFarIO.h"

#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Control.h>

using namespace LOFAR;
using namespace BS;
using namespace LOFAR::GCF;
using namespace LOFAR::GCF::TM;

tNenuFarIO::tNenuFarIO(NenuFarAdmin*	nnfAdmin) : 
	GCFTask((State)&tNenuFarIO::sunshine, "testTask"),
	itsAdmin (nnfAdmin),
	itsTimer (0)
{ 
	itsTimer = new GCFTimerPort(*this, "testTimer");
}

tNenuFarIO::~tNenuFarIO()
{
	delete itsTimer;
}

//
// sunshine
//
GCFEvent::TResult tNenuFarIO::sunshine(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY:
		break;

	case F_INIT: {
		LOG_INFO("### Adding beam_sunshine, active over 5 seconds with duration 7 seconds");
		RTC::Timestamp	ts;
		ts.setNow(5.0);
		IBS_Protocol::Pointing	pointing1(1.11, 2.22, "J2000", ts, 7);
		vector<string>		extraInfo;
		extraInfo.push_back("extraKey1=25");
		extraInfo.push_back("extraKey2=[aap,noot,mies]");
		extraInfo.push_back("hpf=15");
		itsAdmin->addBeam("beam_sunshine", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, extraInfo);
		ASSERTSTR(itsAdmin->nrBeams() == 1, "beamAdmin not correct!");
		LOG_INFO("### Admin OK, waiting 15 seconds...");
		itsTimer->setTimer (15.0);
	} break;

    case F_TIMER: {
		ASSERTSTR(itsAdmin->nrBeams() == 0, "beamAdmin not correct!");
		LOG_INFO("### sunshine test OK");
		TRAN(tNenuFarIO::noAnswer);
	} break;

	case F_EXIT:
		break;

    default:
		LOG_DEBUG_STR ("default:" << eventName(event));
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// noAnswer
//
GCFEvent::TResult tNenuFarIO::noAnswer(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY: {
		LOG_INFO("### Adding beam_noanswer, active over 5 sec, duration 10 sec, requesting no answer");
		RTC::Timestamp	ts;
		ts.setNow(5.0);
		IBS_Protocol::Pointing	pointing1(1.11, 2.22, "J2000", ts, 20);
		vector<string>		extraInfo;
		extraInfo.push_back("extraKey1=25");
		extraInfo.push_back("extraKey2=[aap,noot,mies]");
		extraInfo.push_back("no_answer=true");
		itsAdmin->addBeam("beam_noanswer", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, extraInfo);
		ASSERTSTR(itsAdmin->nrBeams() == 1, "beamAdmin not correct!");
		itsTimer->setTimer(8.0);
		LOG_INFO("### Waiting 8 seconds...");
	} break;

    case F_TIMER: {
		ASSERTSTR(itsAdmin->nrBeams() == 0, "beamAdmin not correct!");
		LOG_INFO("### noAnswer test OK");
		TRAN(tNenuFarIO::testAbort);
	} break;

	case F_EXIT:
		break;

    default:
		LOG_DEBUG_STR ("default:" << eventName(event));
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// testAbort
//
GCFEvent::TResult tNenuFarIO::testAbort(GCFEvent& event, GCFPortInterface& /*port*/)
{
	static	bool	firstTimer(true);

	switch (event.signal) {
	case F_ENTRY: {
		LOG_INFO("### Adding beam_2abort, active over 1 sec, duration 20 sec, aborted after 10 seconds");
		RTC::Timestamp	ts;
		ts.setNow(1.0);
		IBS_Protocol::Pointing	pointing1(1.11, 2.22, "J2000", ts, 20);
		vector<string>		extraInfo;
		extraInfo.push_back("extraKey1=25");
		extraInfo.push_back("extraKey2=[aap,noot,mies]");
		itsAdmin->addBeam("beam_2abort", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, extraInfo);
		itsTimer->setTimer(10.0);
		LOG_INFO("### Waiting 10 seconds...");
	} break;

    case F_TIMER: {
		if (firstTimer) {
			itsAdmin->abortBeam("beam_2abort");
			LOG_INFO("### beam aborted");
			itsTimer->setTimer(2.0);
			firstTimer = false;
		}
		else {
			ASSERTSTR(itsAdmin->nrBeams() == 0, "beamAdmin not correct!");
			LOG_INFO("### beam abort test OK");
			TRAN(tNenuFarIO::testMultipleBeams);
		}
	} break;

	case F_EXIT:
		break;

    default:
		LOG_DEBUG_STR ("default:" << eventName(event));
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// testMultipleBeams
//
GCFEvent::TResult tNenuFarIO::testMultipleBeams(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY: {
		LOG_INFO("### Adding 5 beams, active over 1 sec, duration 10 sec");
		RTC::Timestamp	ts;
		ts.setNow(1.0);
		IBS_Protocol::Pointing	pointing1(1.11, 2.22, "J2000", ts, 10);
		vector<string>		extraInfo;
		extraInfo.push_back("extraKey1=25");
		extraInfo.push_back("extraKey2=[aap,noot,mies]");
		itsAdmin->addBeam("beam_1", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, extraInfo);
		itsAdmin->addBeam("beam_2", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, extraInfo);
		itsAdmin->addBeam("beam_3", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, extraInfo);
		itsAdmin->addBeam("beam_4", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, extraInfo);
		itsAdmin->addBeam("beam_5", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, extraInfo);
		ASSERTSTR(itsAdmin->nrBeams() == 5, "beamAdmin not correct!");
		LOG_INFO("### Waiting 12 seconds");
		itsTimer->setTimer(12.0);
	} break;

    case F_TIMER: {
			ASSERTSTR(itsAdmin->nrBeams() == 0, "beamAdmin not correct!");
			LOG_INFO("### multiple beam test OK");
			TRAN(tNenuFarIO::testAbortAll);
	} break;

	case F_EXIT:
		break;

    default:
		LOG_DEBUG_STR ("default:" << eventName(event));
		break;
	}

	return (GCFEvent::HANDLED);
}
//
// testAbortAll
//
GCFEvent::TResult tNenuFarIO::testAbortAll(GCFEvent& event, GCFPortInterface& /*port*/)
{
	static	bool firstTimer = true;

	switch (event.signal) {
	case F_ENTRY: {
		LOG_INFO("### Adding 5 beams, active over 1 sec, duration 20 sec, aborted after 6 seconds");
		RTC::Timestamp	ts;
		ts.setNow(1.0);
		IBS_Protocol::Pointing	pointing1(1.11, 2.22, "J2000", ts, 20);
		vector<string>		extraInfo;
		extraInfo.push_back("extraKey1=25");
		extraInfo.push_back("extraKey2=[aap,noot,mies]");
		itsAdmin->addBeam("beam_10", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, extraInfo);
		itsAdmin->addBeam("beam_20", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, extraInfo);
		itsAdmin->addBeam("beam_30", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, extraInfo);
		itsAdmin->addBeam("beam_40", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, extraInfo);
		itsAdmin->addBeam("beam_50", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, extraInfo);
		itsTimer->setTimer(6.0);
	} break;

    case F_TIMER: {
		if (firstTimer) {
			itsAdmin->abortAllBeams();
			LOG_INFO("### all beams aborted");
			itsTimer->setTimer(2.0);
			firstTimer = false;
		}
		else {
			ASSERTSTR(itsAdmin->nrBeams() == 0, "beamAdmin not correct!");
			LOG_INFO("### beam abort test OK");
			GCFScheduler::instance()->stop();
		}
	} break;

	case F_EXIT:
		break;

    default:
		LOG_DEBUG_STR ("default:" << eventName(event));
		break;
	}

	return (GCFEvent::HANDLED);
}

// Toplevel test program that can test all the functionality that is related to the NenuFar system.
// It creates the admin-buffer that is the interface for the NenuFar task and then starts the NenuFar task.
// The NenuFar task talks to the tNenuFarStub executable that simulated the NenuFar system.
// 
// Tests can be done by simply injecting beams in the admin-buffer and check the logging of the NenuFar task.
//
// The NenuFarStub can be instructed to make errors in its response by adding fields in the beam info:
// Field             Type      Behaviour
// --------------------------------------------------------------------------------------
// no_answer         trivial   Stub does bot send an answer.
// delay             integer   Nr of seconds the stub should wait before sending an answer
// presume_msgtype   string    Stub acts like it received a message of the type <presume_msgtype>
//
int main(int argc,	char*	argv[]) 
{
	GCFScheduler::instance()->init(argc, argv, LOFAR::basename(argv[0]));

	NenuFarAdmin    theNNFadmin;
	NenuFarIO   nnfTask(&theNNFadmin);
	nnfTask.start();

	tNenuFarIO	testTask(&theNNFadmin);
	testTask.start();

	GCFScheduler::instance()->run();

	return (0);
}


