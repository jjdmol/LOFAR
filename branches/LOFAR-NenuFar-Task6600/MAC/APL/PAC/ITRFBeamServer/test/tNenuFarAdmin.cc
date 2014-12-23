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
//#  $Id: tJ2000Converter.cc 14866 2010-01-23 00:07:02Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>
#include <Common/StreamUtil.h>
#include <ITRFBeamServer/NenuFarAdmin.h>

using namespace LOFAR;
using namespace BS;

void	checkResult (const NenuFarAdmin::BeamInfo& beam, const string& text, const string& expectedName) {
	if (beam.name().empty()) {
		cout << text << beam.name() << endl;
	}
	else {
		cout << text << beam.name() << " bs=" << beam.beamState() << ", cs=" << beam.commState() << endl;
	}
	ASSERTSTR(beam.name() == expectedName, text);
}

int main(int, char*	argv[]) 
{
	INIT_LOGGER(argv[0]);

	IBS_Protocol::Pointing	pointing1(1.11, 2.22, "J2000", RTC::Timestamp(10000, 0), 3000);
	IBS_Protocol::Pointing	pointing2(1.11, 2.22, "J2000", RTC::Timestamp(10050, 0), 3000);
	vector<string>		extraInfo;
	extraInfo.push_back("extraKey1=25");
	extraInfo.push_back("extraKey2=[aap,noot,mies]");

	// --- First test with one beam to see how the firstCommand function works.
	//     add one beam and ask for several timestamps what to communicate.
	//     leave the comm_state on BS_NONE
	// beam1 active from 10000 - 13000
	{
		NenuFarAdmin			nfa;
		NenuFarAdmin::BeamInfo	tmp;
		cout << endl << "Testing with one beam and comm_state == NONE" << endl;
		nfa.addBeam("beam1", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, 15, extraInfo);
		// test around start time.
		tmp	= nfa.firstCommand(10000-5);	checkResult (tmp, "5 sec before start: ", "beam1");
		tmp	= nfa.firstCommand(10000); 		checkResult (tmp, "at starttime      : ", "beam1");
		tmp	= nfa.firstCommand(10000+5);	checkResult (tmp, "5 sec after start : ", "beam1");
		// test around stoptime
		tmp	= nfa.firstCommand(13000-5);	checkResult (tmp, "5 sec before end  : ", "beam1");
		tmp	= nfa.firstCommand(13000); 		checkResult (tmp, "at endtime        : ", "");
		tmp	= nfa.firstCommand(13000+5);	checkResult (tmp, "5 sec after end   : ", "");
		nfa.abortBeam("beam1");	// beam already removed, should not cause an error.
		cout << nfa << endl;
	}

	// --- Same test but now with communication state set to BS_NEW
	{
		NenuFarAdmin			nfa;
		NenuFarAdmin::BeamInfo	tmp;
		cout << endl << "Testing with one beam and comm_state == NEW" << endl;
		nfa.addBeam("beam1", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, 15, extraInfo);
		ASSERTSTR(nfa.setCommState("beam1", NenuFarAdmin::BeamInfo::BS_NEW), "Setting state of beam1 to NEW failed");
		// test around start time.
		tmp	= nfa.firstCommand(10000-5);	checkResult (tmp, "5 sec before start: ", "");
		tmp	= nfa.firstCommand(10000); 		checkResult (tmp, "at starttime      : ", "");
		tmp	= nfa.firstCommand(10000+5);	checkResult (tmp, "5 sec after start : ", "");
		// test around stoptime
		tmp	= nfa.firstCommand(13000-5);	checkResult (tmp, "5 sec before end  : ", "");
		tmp	= nfa.firstCommand(13000); 		checkResult (tmp, "at endtime        : ", "beam1");
		tmp	= nfa.firstCommand(13000+5);	checkResult (tmp, "5 sec after end   : ", "beam1");
		cout << nfa << endl;
	}

	// --- Same test but now with communication state set to BS_NEW
	{
		NenuFarAdmin			nfa;
		NenuFarAdmin::BeamInfo	tmp;
		cout << endl << "Testing with one beam and comm_state == ENDED" << endl;
		nfa.addBeam("beam1", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, 15, extraInfo);
		ASSERTSTR(nfa.setCommState("beam1", NenuFarAdmin::BeamInfo::BS_ENDED), "Setting state of beam1 to ENDED failed");
		// test around start time.
		tmp	= nfa.firstCommand(10000-5);	checkResult (tmp, "5 sec before start: ", "");	// impossible
		tmp	= nfa.firstCommand(10000); 		checkResult (tmp, "at starttime      : ", "");	// impossible
		tmp	= nfa.firstCommand(10000+5);	checkResult (tmp, "5 sec after start : ", "");	// impossible
		// test around stoptime
		tmp	= nfa.firstCommand(13000-5);	checkResult (tmp, "5 sec before end  : ", "");	// impossible
		tmp	= nfa.firstCommand(13000); 		checkResult (tmp, "at endtime        : ", "");
		tmp	= nfa.firstCommand(13000+5);	checkResult (tmp, "5 sec after end   : ", "");
		cout << nfa << endl;
	}

	// --- Test with two beams in commState NONE
	{
		// beam1 active from 10000 - 13000
		// beam2 active from 10050 - 13050
		NenuFarAdmin			nfa;
		NenuFarAdmin::BeamInfo	tmp;
		cout << endl << "Testing with two beams and comm_state == NONE" << endl;
		nfa.addBeam("beam1", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, 15, extraInfo);
		nfa.addBeam("beam2", "LBA_OUTER", bitset<192>(0x0FDE37F), 1, pointing2, 15, extraInfo);
		tmp	= nfa.firstCommand(10030); 		checkResult (tmp, "between both starts:", "beam1");
		tmp	= nfa.firstCommand(10030); 		checkResult (tmp, "and again...       :", "beam1");
		tmp	= nfa.firstCommand(10075); 		checkResult (tmp, "when both active   :", "beam1");
		tmp	= nfa.firstCommand(13022); 		checkResult (tmp, "after beam1 ended  :", "beam2");
		tmp	= nfa.firstCommand(13062); 		checkResult (tmp, "after beam2 ended  :", "");
	}

	// --- Test with two beams with beam1 in commState NEW
	{
		// beam1 active from 10000 - 13000
		// beam2 active from 10050 - 13050
		NenuFarAdmin			nfa;
		NenuFarAdmin::BeamInfo	tmp;
		cout << endl << "Testing with two beams in resp. comm_state NEW and NONE" << endl;
		nfa.addBeam("beam1", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, 15, extraInfo);
		nfa.addBeam("beam2", "LBA_OUTER", bitset<192>(0x0FDE37F), 1, pointing2, 15, extraInfo);
		ASSERTSTR(nfa.setCommState("beam1", NenuFarAdmin::BeamInfo::BS_NEW), "Setting state of beam1 to NEW failed");
		tmp	= nfa.firstCommand(10030); 		checkResult (tmp, "between both starts:", "beam2");
		tmp	= nfa.firstCommand(10030); 		checkResult (tmp, "and again...       :", "beam2");
		tmp	= nfa.firstCommand(10075); 		checkResult (tmp, "when both active   :", "beam2");
		tmp	= nfa.firstCommand(13022); 		checkResult (tmp, "after beam1 ended  :", "beam1");
		tmp	= nfa.firstCommand(13062); 		checkResult (tmp, "after beam2 ended  :", "beam1");
	}

	// --- Testing ABORT with 1 beam in state NONE
	{
		// beam1 active from 10000 - 13000
		NenuFarAdmin			nfa;
		NenuFarAdmin::BeamInfo	tmp;
		cout << endl << "Testing abort with one beam in NONE" << endl;
		nfa.addBeam("beam1", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, 15, extraInfo);
		tmp	= nfa.firstCommand(10000-5); 	checkResult (tmp, "NONE before start   :", "beam1");
		tmp	= nfa.firstCommand(10000+5);	checkResult (tmp, "NONE after start    :", "beam1");
		ASSERTSTR(nfa.abortBeam("beam1"), "Aborting beam1");
		tmp	= nfa.firstCommand(10000-5); 	checkResult (tmp, "Aborted,before start:", "");
		tmp	= nfa.firstCommand(10000+5);	checkResult (tmp, "Aborted,after start :", "");
	}

	// --- Testing ABORT with 1 beam in state NEW
	{
		// beam1 active from 10000 - 13000
		NenuFarAdmin			nfa;
		NenuFarAdmin::BeamInfo	tmp;
		cout << endl << "Testing abort with 1 beam in NEW" << endl;
		nfa.addBeam("beam1", "LBA_INNER", bitset<192>(0x0FF0FFF), 1, pointing1, 15, extraInfo);
		ASSERTSTR(nfa.setCommState("beam1", NenuFarAdmin::BeamInfo::BS_NEW), "Setting state of beam1 to NEW failed");
		tmp	= nfa.firstCommand(10000-5); 	checkResult (tmp, "NEW before start    :", "");
		tmp	= nfa.firstCommand(10000+5);	checkResult (tmp, "NEW after start     :", "");
		ASSERTSTR(nfa.abortBeam("beam1"), "Aborting beam1");
		tmp	= nfa.firstCommand(10000-5); 	checkResult (tmp, "Aborted,before start:", "beam1");
		tmp	= nfa.firstCommand(10000+5);	checkResult (tmp, "Aborted,after start :", "beam1");
	}

}
