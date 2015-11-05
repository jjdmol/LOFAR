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
#include <ITRFBeamServer/NenuFarMsg.h>

using namespace LOFAR;
using namespace BS;

int main(int, char*	argv[]) 
{
	INIT_LOGGER(argv[0]);

	vector<string>	payload;
	payload.push_back("antennaSet=NENUFAR");
	payload.push_back("hpf=15");

	NenuFarMsg	msg1(0x102, NEW_BEAM_MSG, payload);
	cout <<  msg1 << endl;

	ParameterSet	ps;
	ps.add("antennaSet", "NENUFAR");
	ps.add("hpf", "15");

	NenuFarMsg	msg2(0x102, NEW_BEAM_MSG, ps);
	cout << msg2 << endl;

	ASSERTSTR(msg1 == msg2, "vector version differs from ParameterSet version");

	ps.add("rcus", "0:191");
	NenuFarMsg	msg3(0x102, NEW_BEAM_MSG, ps);
	cout << msg3 << endl;
	ASSERTSTR(msg1 != msg3, "vector version should be different from ParameterSet version");
	ASSERTSTR(msg3 != msg1, "vector version should be different from ParameterSet version");

	vector<string>	daolyap = NenuFarMsg::unpack2vector(msg1.data(), msg1.size());
	cout << daolyap << endl;

	ParameterSet	sp = NenuFarMsg::unpack2parset(msg2.data(), msg2.size());
	cout << sp << endl;

	vector<string>	emptyvector;
	vector<string>	minimal;
	minimal.push_back("beamName=testBeam");

	NenuFarMsg	msg01(0x100, NEW_BEAM_MSG, minimal);
	NenuFarMsg	msg02(0x100, NEW_BEAM_ACK_MSG, minimal);
	NenuFarMsg	msg03(0x100, STOP_BEAM_MSG, minimal);
	NenuFarMsg	msg04(0x100, STOP_BEAM_ACK_MSG, minimal);
	NenuFarMsg	msg05(0x100, ABORT_BEAM_MSG, minimal);
	NenuFarMsg	msg06(0x100, ABORT_BEAM_ACK_MSG, minimal);
	NenuFarMsg	msg07(0x100, ABORT_ALL_BEAMS_MSG, emptyvector);
	cout << msg01 << endl;
	cout << msg02 << endl;
	cout << msg03 << endl;
	cout << msg04 << endl;
	cout << msg05 << endl;
	cout << msg06 << endl;
	cout << msg07 << endl;
}
