//#
//#  BeamServerTest.cc: class definition for the Beam Server task.
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>

#include "Beams.h"
#include "BSTest.h"
#include <APL/RSP_Protocol/MEPHeader.h>
#include <APL/RTCCommon/Timestamp.h>
#include <APL/CAL_Protocol/SubArray.h>

#include <GCF/TM/GCF_Control.h>
#include <APL/RTCCommon/PSAccess.h>

#define CLIENT
#ifdef CLIENT
#include <AMCBase/ConverterClient.h>
#else
#include <AMCImpl/ConverterImpl.h>
#endif

#include <iostream>

#include <blitz/array.h>

#define TEST_N_BEAMS              8
#define TEST_N_BEAMLETS           256 // MEPHeader::N_BEAMLETS
#define TEST_N_SUBBANDS_PER_BEAM  (TEST_N_BEAMLETS/TEST_N_BEAMS)
#define TEST_N_LOOPS              4

#define MAX_N_BEAMS              256
#define MAX_COMPUTE_INTERVAL     16

using namespace blitz;
using namespace std;
using namespace LOFAR;
using namespace BS;
using namespace EPA_Protocol;
using namespace BS_Protocol;
using namespace RTC;
using namespace CAL;

#define COMPUTE_INTERVAL 5
#define N_ELEMENTS       96
#define N_POLARIZATIONS  2
#define SIMULATE_SECONDS 2 * MAX_COMPUTE_INTERVAL

namespace LOFAR {

class BeamServerTest : public Test 
{
private:
	Beams m_beams;
	Beam* m_beamptr[TEST_N_BEAMLETS];

public:
	BeamServerTest() :
		Test("BeamServerTest"),
		m_beams(TEST_N_BEAMLETS,
		MEPHeader::N_SUBBANDS)
	{}

	void run() {
#if 0
		allocate();
		deallocate();
		subbandSelection();
		oneTooManyBeam();
		oneTooManyBeamlet();
		emptyBeam();
		pointing();
#endif
		convert_pointings();
	}

	void allocate() {
		Beamlet2SubbandMap allocation;

		for (int bi = 0; bi < TEST_N_BEAMS; bi++) {
			allocation().clear();
			for (int si = 0; si < TEST_N_SUBBANDS_PER_BEAM; si++) {
				allocation()[si + (TEST_N_SUBBANDS_PER_BEAM * bi)] = si;
			}

			char nodeid[32];
			snprintf(nodeid, 32, "beam.%d", bi);

			TESTC(0 != (m_beamptr[bi] = m_beams.create(nodeid, "unspecified", allocation)));
		}
	}

	void deallocate() {
		for (int i = 0; i < TEST_N_BEAMS; i++) {
			TESTC(true == m_beams.destroy(m_beamptr[i]));
		}
	}

	void subbandSelection() {
		START_TEST("subbandsSelection", "test subband selection");

		allocate();

		Beamlet2SubbandMap selection;
		selection = m_beams.getSubbandSelection();

		TESTC(selection().size() == TEST_N_BEAMS * TEST_N_SUBBANDS_PER_BEAM);

		deallocate();

		STOP_TEST();
	}

	void oneTooManyBeam() {
		START_TEST("oneTooManyBeam", "test if allocation of one more beam than possible does indeed fail");

		allocate();

		// and allocate one more
		Beam* beam = 0;

		Beamlet2SubbandMap allocation;

		for (int i = 0; i < TEST_N_SUBBANDS_PER_BEAM; i++) {
			allocation()[i] = i;
		}

		TESTC(0 == (beam = m_beams.create("beam.0", "unspecified", allocation)));

		deallocate();

		STOP_TEST();
	}

	void oneTooManyBeamlet() {
		START_TEST("oneTooManyBeamlet", "test if allocation of one more subband fails");

		Beamlet2SubbandMap allocation;

		for (int i = 0; i < TEST_N_BEAMLETS + 1; i++) {
			allocation()[i] = i;
		}

		TESTC(0 == (m_beamptr[0] = m_beams.create("beam.100", "unspecified", allocation)));

		STOP_TEST();
	}

	void emptyBeam() {
		START_TEST("emptyBeam", "check that empty beam allocation fails");

		Beamlet2SubbandMap allocation;

		TESTC(0 == (m_beamptr[0] = m_beams.create("beam.0", "unspecified", allocation)));
		TESTC(false == m_beams.destroy(m_beamptr[0]));

		STOP_TEST();
	}

	void pointing() {
		START_TEST("pointing", "check addPointing on allocated and deallocated beam");

		Beamlet2SubbandMap allocation;
		for (int i = 0; i < TEST_N_BEAMLETS; i++) {
			allocation()[i] = i;
		}

		// allocate beam, addPointing, should succeed
		TESTC(0 != (m_beamptr[0] = m_beams.create("beam.0", "unspecified", allocation)));
		if (m_beamptr[0]) {
			m_beamptr[0]->addPointing(Pointing(0.0, 0.0, Timestamp::now(20), Pointing::J2000));
		}

		TESTC(true == m_beams.destroy(m_beamptr[0]));

		STOP_TEST();
	}

	void convert_pointings() {
		START_TEST("convert_pointings", "convert pointings and calculate weights");

		Range all = Range::all();

		Beamlet2SubbandMap allocation;

//       for (int i = 0; i < TEST_N_BEAMLETS; i++) {
// 	allocation()[i] = i;
//       }

		Array<double, 3>          pos(N_ELEMENTS, N_POLARIZATIONS, 3);
		Array<bool, 2>            select(N_ELEMENTS, N_POLARIZATIONS);
		Array<double, 1> loc(3);

		pos = 1.0; // x,y coordiante = 1
		pos(all, all, 2) = 0.0; // z-coordinate = 0

		loc = 0.0, 0.0, 0.0;
		select = true;
		SubArray subarray("subarray", loc, pos, select, 160000000.0, 1, MEPHeader::N_SUBBANDS, 0xB0 /* LBA */);

#ifdef CLIENT
		AMC::ConverterClient* converter = 0;
		try { converter = new AMC::ConverterClient("localhost"); }
		catch (Exception& e) {
			LOG_FATAL("Failed to connect to amcserver on localhost");
			exit(EXIT_FAILURE);
		}
#else
		AMC::ConverterImpl* converter = new AMC::ConverterImpl();
#endif

		for (int nbeams = 1; nbeams <= MAX_N_BEAMS; nbeams *= 2) {
			//if (nbeams > TEST_N_BEAMLETS) nbeams = TEST_N_BEAMLETS;
			for (int compute_interval = 1; compute_interval <= MAX_COMPUTE_INTERVAL; compute_interval *= 2) {

				int nsubbands_per_beam = TEST_N_BEAMLETS / nbeams;

				Array<complex<double>, 3> weights(compute_interval, N_ELEMENTS * N_POLARIZATIONS, TEST_N_BEAMLETS);

				// mark the time
				Timestamp now = Timestamp::now();

				bool alloc_ok = true;
				for (int bi = 0; bi < nbeams; bi++) {
					allocation().clear();
					for (int si = 0; si < nsubbands_per_beam; si++) {
						allocation()[si + (nsubbands_per_beam * bi)] = si;
					}

					char nodeid[32];
					snprintf(nodeid, 32, "beam.%d", bi);

					if (0 == (m_beamptr[bi] = m_beams.create(nodeid, "unspecified", allocation))) {
						alloc_ok = false;
					}

					// add a few pointings

					//3C461 Cassiopeia A
					m_beamptr[bi]->addPointing(Pointing(6.123662, 1.026719, RTC::Timestamp::now(0), Pointing::J2000));
					// 3C405 Cygnus A
					m_beamptr[bi]->addPointing(Pointing(5.233748, 0.711018, RTC::Timestamp::now(3), Pointing::J2000));
					// 3C144 Crab nebula (NGC 1952)
					m_beamptr[bi]->addPointing(Pointing(1.459568, 0.384089, RTC::Timestamp::now(5), Pointing::J2000));
					// 3C274 Virgo NGC4486(M87)
					m_beamptr[bi]->addPointing(Pointing(3.276114, 0.216275, RTC::Timestamp::now(8), Pointing::J2000));

					m_beamptr[bi]->setSubarray(subarray);
				}
				TESTC(alloc_ok);

				// start timer
				struct timeval start, delay;
				gettimeofday(&start, 0);

				printf("\n");
				int loop = 0;
				for (loop = 0; loop < TEST_N_LOOPS; loop ++) {
					// calculate_weights
					m_beams.calculate_weights(now + (long)loop, compute_interval, converter, weights);

					printf("\r%d/%d", loop, TEST_N_LOOPS); fflush(stdout);
				}
				printf("\r%d/%d done\n", loop, loop);

				// stop timer
				gettimeofday(&delay, 0);
				delay.tv_sec -= start.tv_sec;
				delay.tv_usec -= start.tv_usec;
				if (delay.tv_usec < 0) {
					delay.tv_sec -= 1;
					delay.tv_usec = 1000000 + delay.tv_usec;
				}

				// average over loop times
				delay.tv_usec += delay.tv_sec * 1000000;
				delay.tv_usec /= loop;

				// convert back to sec.usec
				delay.tv_sec = delay.tv_usec / 1000000;
				delay.tv_usec -= (delay.tv_sec * 1000000);

				printf("\nnbeams\tnsubbands_per_beam\tcompute_interval\tavg. time\n");
				printf("%d\t\t%d\t\t\t%d\t\t%ld.%03ld\n", nbeams, nsubbands_per_beam, compute_interval, delay.tv_sec, delay.tv_usec/1000);
				printf("\n");

				bool destroy_ok = true;
				for (int i = 0; i < nbeams; i++) {
					if (!m_beams.destroy(m_beamptr[i])) {
						destroy_ok = false;
					}
				}
				TESTC(destroy_ok);
			}
		}

		if (converter) {
			delete converter;
		}

		STOP_TEST();
	}

};// class beamservertest

};	// namespace LOFAR

int main(int argc, char** argv)
{
	GCFTask::init(argc, argv);

	LOG_INFO(formatString("Program %s has started", argv[0]));

	try {
		ConfigLocator cl;
		globalParameterSet()->adoptFile(cl.locate("BeamServer.conf"));
	}
	catch (Exception& e) {
		cerr << "Failed to load configuration files: " << e.text() << endl;
		exit(EXIT_FAILURE);
	}

	Suite s("Beam Server Test Suite", &cout);

	s.addTest(new BeamServerTest);
	s.run();
	long nFail = s.report();
	s.free();

	LOG_INFO(formatString("Normal termination of program %s", argv[0]));

	return nFail;
}
